#pragma once

#include <dxil2spirv.h>
#include <dxc/DXIL/DXIL.h>
#include <map>
#include "../SPIRV-Builder/Module.h"

namespace dxil2spirv
{
	namespace dxil
	{
		class Context
		{
		public:
			Context(const options* globalOptions, hlsl::DxilModule& dxilModule);

			const options* getGlobalOptions() const { return globalOptions; }
			hlsl::DxilModule& getDxilModule() { return dxilModule; }
			spirv::Module& getSpirvModule() { return spvModule; }

			spirv::Handle nextHandle() { return getSpirvModule().nextHandle(); }

			spirv::Handle findStructType(const hlsl::DxilStructAnnotation* structAnnotation);
			spirv::Handle findCompType(hlsl::CompType compType);
			spirv::Handle findArrayType(spirv::Handle baseType, uint32_t size);
			spirv::Handle findVectorType(spirv::Handle baseType, uint32_t size);
			spirv::Handle findMatrixType(hlsl::CompType compType,
				const hlsl::DxilMatrixAnnotation& matrixAnnotation);
			spirv::Handle findVoidType();
		private:
			const options* globalOptions;
			hlsl::DxilModule& dxilModule;
			spirv::Module spvModule;

			spirv::Handle voidTypeCache;
			std::map<const hlsl::DxilStructAnnotation*, spirv::Handle> structTypeCache;
			std::map<hlsl::CompType::Kind, spirv::Handle> compTypeCache;
			std::map<std::tuple<spirv::Handle, uint32_t>, spirv::Handle> arrayTypeCache, vectorTypeCache;
			std::map<std::tuple<hlsl::CompType::Kind, unsigned int, unsigned int, hlsl::MatrixOrientation>, 
				spirv::Handle> matrixTypeCache;
		};

		void convert(const options* options,
			hlsl::DxilModule& dxilModule, std::vector<uint32_t>& spirv);
	}
}