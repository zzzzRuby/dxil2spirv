#include <dxil2spirv.h>
#include <dxil2spirv_test.h>

#if defined(_WIN32)
#	include <Windows.h>
#	include <atlbase.h>
#endif

#include <dxc/dxcapi.h>
#include <algorithm>
#include <fstream>
#include <iostream>

#include <spirv-tools/libspirv.hpp>

int main()
{
	CComPtr<IDxcCompiler> pCompiler;
	HRESULT hr = DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&pCompiler));
	CComPtr<IDxcLibrary> pLibrary;
	hr = DxcCreateInstance(CLSID_DxcLibrary, IID_PPV_ARGS(&pLibrary));

	for each (const dxil2spirv_test & test in dxil2spirv_test_instances)
	{
		CComPtr<IDxcBlobEncoding> pSource;
		pLibrary->CreateBlobWithEncodingFromPinned(test.hlsl.data(),
			static_cast<UINT32>(test.hlsl.size()), CP_UTF8, &pSource);

		CComPtr<IDxcOperationResult> pResult;
		HRESULT hr = pCompiler->Compile(
			pSource,
			L"dxil2spirv_test",
			test.entry.data(),
			test.profile.data(),
			nullptr,
			0,
			nullptr,
			0,
			nullptr,
			&pResult);
		if (FAILED(hr))
		{
			break;
		}

		HRESULT compileResult;
		pResult->GetStatus(&compileResult);

		if (FAILED(compileResult))
		{
			CComPtr<IDxcBlobEncoding> pError;
			pResult->GetErrorBuffer(&pError);

			std::cout << std::string((const char*)pError->GetBufferPointer(), pError->GetBufferSize()) << std::endl;
			break;
		}

		CComPtr<IDxcBlob> pDXIL;
		pResult->GetResult(&pDXIL);

		std::vector<uint32_t> spirv;
		dxil2spirv::options opt;
		opt.stripName = false;
		opt.runValidator = false;
		opt.enableOptimizer = false;
		dxil2spirv::convert(spirv,
			static_cast<const uint8_t*>(pDXIL->GetBufferPointer()),
			static_cast<size_t>(pDXIL->GetBufferSize()),
			&opt);

		std::string disSpirv;
		spvtools::SpirvTools tools(spv_target_env::SPV_ENV_VULKAN_1_1);
		tools.Disassemble(spirv, &disSpirv);

		std::ofstream(std::string(test.name).append(".gen.spv"), std::ios::binary).write(
			(const char*)spirv.data(),
			spirv.size() * sizeof(uint32_t)
		);
		std::ofstream(std::string(test.name).append(".gen.spv.asm")) << disSpirv;
	}
    return 0;
}