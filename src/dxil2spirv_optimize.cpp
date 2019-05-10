#include <dxil2spirv.h>
#include <spirv-tools/optimizer.hpp>

namespace dxil2spirv
{
	void optimize(const options* _option, std::vector<uint32_t>& spirv, spv_target_env spvEnv)
	{
		spvtools::Optimizer optimizer(spvEnv);
		std::vector<uint32_t> optimized_spirv;
		if (_option->stripName)
			optimizer.RegisterPass(spvtools::CreateStripDebugInfoPass());
		if (_option->enableOptimizer)
		{
			optimizer.RegisterPerformancePasses();
			optimizer.RegisterLegalizationPasses();
			optimizer.RegisterSizePasses();
			optimizer.RegisterPass(spvtools::CreateFlattenDecorationPass());
			optimizer.RegisterPass(spvtools::CreateUnifyConstantPass());
			optimizer.RegisterPass(spvtools::CreateReplaceInvalidOpcodePass());
			optimizer.RegisterPass(spvtools::CreateFoldSpecConstantOpAndCompositePass());
			optimizer.RegisterPass(spvtools::CreateInsertExtractElimPass());
			optimizer.RegisterPass(spvtools::CreateCompactIdsPass());
		}

		if (optimizer.Run(spirv.data(), spirv.size(), &optimized_spirv))
			std::swap(optimized_spirv, spirv);
	}
}