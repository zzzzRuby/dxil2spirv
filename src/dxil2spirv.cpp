#include <iostream>
#include <sstream>
#include "DXIL-Translator/Translator.h"
#include <dxc/DxilContainer/DxilContainer.h>
#include <llvm/Bitcode/ReaderWriter.h>
#include <llvm/IR/LLVMContext.h>
#include <spirv-tools/libspirv.hpp>

extern "C"
{
	const dxil2spirv_options* dxil2spirv_get_default_options();

	struct dxil2spirv_result
	{
		std::vector<uint32_t> data;
	};

	dxil2spirv_result* dxil2spirv_convert(const uint8_t* dxil,
		size_t dxilSize, const dxil2spirv_options* option)
	{
		dxil2spirv_result* ret = new dxil2spirv_result();
		dxil2spirv::convert(ret->data, dxil, dxilSize, option);
		return ret;
	}

	void dxil2spirv_free(dxil2spirv_result* result) { delete result; }

	size_t dxil2spirv_result_size(dxil2spirv_result* result)
	{
		return result->data.size();
	}

	uint32_t* dxil2spirv_result_data(dxil2spirv_result* result)
	{
		return result->data.data();
	}
}

namespace dxil2spirv
{
	void optimize(const options* _option, std::vector<uint32_t>& spirv, spv_target_env spvEnv);

	static bool DxilFindModule(const uint8_t* dxil, size_t dxilSize, std::vector<uint8_t>& pTarget) {

		const hlsl::DxilContainerHeader* pContainer = hlsl::IsDxilContainerLike(dxil, dxilSize);

		if (pContainer == nullptr) return false;
		if (!hlsl::IsValidDxilContainer(pContainer, dxilSize)) return false;

		hlsl::DxilPartIterator it = std::find_if(begin(pContainer), end(pContainer),
			hlsl::DxilPartIsType(hlsl::DFCC_DXIL));

		if (it == end(pContainer)) return false;

		const char* pIL = nullptr;
		uint32_t ILLength = 0;
		hlsl::GetDxilProgramBitcode(
			reinterpret_cast<const hlsl::DxilProgramHeader*>(hlsl::GetDxilPartData(*it)), &pIL,
			&ILLength);

		pTarget.resize(ILLength);
		memcpy(pTarget.data(), pIL, pTarget.size());

		return true;
	}

	void convert(std::vector<uint32_t> & spirv, const uint8_t * dxil,
		size_t dxilSize, const options * option)
	{
		constexpr spv_target_env spvEnv = spv_target_env::SPV_ENV_VULKAN_1_1;

		std::vector<uint8_t> mDXIL;
		DxilFindModule(dxil, dxilSize, mDXIL);

		std::unique_ptr<llvm::MemoryBuffer> pBitcodeBuf =
			llvm::MemoryBuffer::getMemBuffer(
				llvm::StringRef((char*)mDXIL.data(), mDXIL.size()), "", false);
		auto MorE = llvm::parseBitcodeFile(pBitcodeBuf->getMemBufferRef(),
			llvm::getGlobalContext());

		auto err = MorE.getError();
		if (MorE)
		{
			std::unique_ptr<llvm::Module> M = std::move(MorE.get());
			hlsl::DxilModule& dxilM = M->GetOrCreateDxilModule();

			const options* _option =
				option == nullptr ? dxil2spirv_get_default_options() : option;

			dxil::convert(_option, dxilM, spirv);

			if (_option->runValidator)
			{
				spvtools::SpirvTools tools(spvEnv);
				std::stringstream messageConsumer;
				tools.SetMessageConsumer([&messageConsumer](spv_message_level_t level, const char*, const spv_position_t & position, const char* message)
				{
					switch (level)
					{
					case SPV_MSG_FATAL:
					case SPV_MSG_INTERNAL_ERROR:
					case SPV_MSG_ERROR:
						messageConsumer << "error: line " << position.index << ": " << message
							<< std::endl;
						break;
					case SPV_MSG_WARNING:
						messageConsumer << "warning: line " << position.index << ": " << message
							<< std::endl;
						break;
					case SPV_MSG_INFO:
						messageConsumer << "info: line " << position.index << ": " << message
							<< std::endl;
						break;
					default:
						break;
					}
				});

				bool succeed = tools.Validate(spirv.data(), spirv.size());

				if (std::string str = messageConsumer.str(); str != "")
				{
					//TODO: output str
				}

				if (!succeed) return;
			}

			optimize(_option, spirv, spvEnv);
		}
	}
}