#define _HAS_AUTO_PTR_ETC 1
#define _SILENCE_CXX17_ITERATOR_BASE_CLASS_DEPRECATION_WARNING

#include <dxil2spirv.h>
#include <iostream>
#include "DXIL-Translator/Translator.h"
#include <dxc/DXIL/DXIL.h>
#include <dxc/DxilContainer/DxilContainer.h>
#include <llvm/IR/BasicBlock.h>
#include <llvm/ADT/APFloat.h>
#include <llvm/Bitcode/ReaderWriter.h>
#include <llvm/IR/LLVMContext.h>

namespace dxil2spirv
{
	void optimize(const options* options, std::vector<uint32_t>& spirv);
}

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

	static void convert(const options * options,
		const hlsl::DxilModule & dxilModule, std::vector<uint32_t> & spirv)
	{

	}

	void convert(std::vector<uint32_t> & spirv, const uint8_t * dxil,
		size_t dxilSize, const options * option) {
		std::vector<uint8_t> mDXIL;
		DxilFindModule(dxil, dxilSize, mDXIL);

		std::unique_ptr<llvm::MemoryBuffer> pBitcodeBuf =
			llvm::MemoryBuffer::getMemBuffer(
				llvm::StringRef((char*)mDXIL.data(), mDXIL.size()), "", false);
		auto MorE = llvm::parseBitcodeFile(pBitcodeBuf->getMemBufferRef(),
			llvm::getGlobalContext());

		auto err = MorE.getError();
		if (MorE) {
			std::unique_ptr<llvm::Module> M = std::move(MorE.get());
			hlsl::DxilModule& dxilM = M->GetOrCreateDxilModule();

			const options* _option =
				option == nullptr ? dxil2spirv_get_default_options() : option;

			convert(_option, dxilM, spirv);
			optimize(_option, spirv);
		}
	}
}