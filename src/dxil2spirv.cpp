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

extern "C" {
	const dxil2spirv_options* dxil2spirv_get_default_options();
}

namespace dxil2spirv
{
	static bool DxilFindModule(hlsl::DxilFourCC fourCC, const uint8_t* dxil,
		size_t dxilSize, std::vector<uint8_t>& pTarget) {
		const uint32_t BC_C0DE = ((int32_t)(int8_t)'B' | (int32_t)(int8_t)'C' << 8 |
			(int32_t)0xDEC0 << 16); // BC0xc0de in big endian
		const char* pBitcode = nullptr;
		const hlsl::DxilPartHeader * pDxilPartHeader = (hlsl::DxilPartHeader*)dxil;
		if (BC_C0DE == *(uint32_t*)dxil) {
			pTarget.resize(dxilSize);
			memcpy(pTarget.data(), dxil, dxilSize);
			return true;
		}
		if (hlsl::IsValidDxilContainer((hlsl::DxilContainerHeader*)dxil, dxilSize)) {
			hlsl::DxilContainerHeader* pDxilContainerHeader =
				(hlsl::DxilContainerHeader*)dxil;
			pDxilPartHeader =
				*std::find_if(begin(pDxilContainerHeader), end(pDxilContainerHeader),
					hlsl::DxilPartIsType(fourCC));
		}
		if (fourCC == pDxilPartHeader->PartFourCC) {
			uint32_t pBlobSize;
			hlsl::DxilProgramHeader* pDxilProgramHeader =
				(hlsl::DxilProgramHeader*)(pDxilPartHeader + 1);
			hlsl::GetDxilProgramBitcode(pDxilProgramHeader, &pBitcode, &pBlobSize);
			pTarget.resize(pBlobSize);
			memcpy(pTarget.data(), pBitcode, pTarget.size());

			return true;
		}
		return false;
	}

	static void convert(const options * options,
		const hlsl::DxilModule & dxilModule, std::vector<uint32_t> & spirv)
	{

	}

	void convert(std::vector<uint32_t> & spirv, const uint8_t * dxil,
		size_t dxilSize, const options * option) {
		std::vector<uint8_t> mDXIL;
		DxilFindModule(hlsl::DFCC_DXIL, dxil, dxilSize, mDXIL);

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

extern "C"
{
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