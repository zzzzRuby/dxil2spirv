#include "Translator.h"
#include <llvm/IR/DerivedTypes.h>

namespace dxil2spirv
{
	namespace dxil
	{
		void ConvertTypes(Context& ctx);
		void ConvertResources(Context& ctx);
		void ConvertInputs(Context& ctx);
		void ConvertOutputs(Context& ctx);
		void ConvertFunctions(Context& ctx);

		Context::Context(const options* globalOptions, hlsl::DxilModule& dxilModule)
			: globalOptions(globalOptions), dxilModule(dxilModule),
			  spvModule(spv::AddressingModel::Logical, spv::MemoryModel::GLSL450)
		{
			spvModule.addCapability(spv::Capability::Shader);
		}

		static inline void convert(Context& ctx)
		{
			ConvertTypes(ctx);
			ConvertResources(ctx);
			ConvertFunctions(ctx);
		}

		void convert(const options* options, hlsl::DxilModule& dxilModule, std::vector<uint32_t>& spirv)
		{
			Context ctx(options, dxilModule);
			convert(ctx);
			ctx.getSpirvModule().dump(spirv);
		}
	}
}