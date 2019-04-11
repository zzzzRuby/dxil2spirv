#include <dxil2spirv.h>
#include <spirv-tools/optimizer.hpp>

namespace dxil2spirv
{
	template <spvtools::Optimizer::PassToken(*PassFunc)()>
	static inline void doOptimize(std::vector<uint32_t>& spirv) {
		std::vector<uint32_t> optimized_spirv;

		spvtools::Optimizer optimizer(spv_target_env::SPV_ENV_VULKAN_1_1);

		optimizer.RegisterPass(PassFunc());

		if (optimizer.Run(spirv.data(), spirv.size(), &optimized_spirv))
			std::swap(optimized_spirv, spirv);
	}

	static void doOptimize(std::vector<uint32_t>& spirv,
		spvtools::Optimizer::PassToken&& token) {
		std::vector<uint32_t> optimized_spirv;

		spvtools::Optimizer optimizer(spv_target_env::SPV_ENV_VULKAN_1_1);

		optimizer.RegisterPass(std::move(token));

		if (optimizer.Run(spirv.data(), spirv.size(), &optimized_spirv))
			std::swap(optimized_spirv, spirv);
	}

	static void doPerformanceOptimize(std::vector<uint32_t>& spirv) {
		std::vector<uint32_t> optimized_spirv;

		spvtools::Optimizer optimizer(spv_target_env::SPV_ENV_VULKAN_1_1);

		optimizer.RegisterPerformancePasses();

		if (optimizer.Run(spirv.data(), spirv.size(), &optimized_spirv))
			std::swap(optimized_spirv, spirv);
	}

	static void doLegalizationOptimize(std::vector<uint32_t>& spirv) {
		std::vector<uint32_t> optimized_spirv;

		spvtools::Optimizer optimizer(spv_target_env::SPV_ENV_VULKAN_1_1);

		optimizer.RegisterLegalizationPasses();

		if (optimizer.Run(spirv.data(), spirv.size(), &optimized_spirv))
			std::swap(optimized_spirv, spirv);
	}

	static void doSizeOptimize(std::vector<uint32_t>& spirv) {
		std::vector<uint32_t> optimized_spirv;

		spvtools::Optimizer optimizer(spv_target_env::SPV_ENV_VULKAN_1_1);

		optimizer.RegisterSizePasses();

		if (optimizer.Run(spirv.data(), spirv.size(), &optimized_spirv))
			std::swap(optimized_spirv, spirv);
	}

	void optimize(const options* _option, std::vector<uint32_t>& spirv) {
		if (_option->stripName)
			doOptimize<spvtools::CreateStripDebugInfoPass>(spirv);
		if (_option->enableOptimizer) {
			doPerformanceOptimize(spirv);
			doLegalizationOptimize(spirv);
			doSizeOptimize(spirv);
			doOptimize<spvtools::CreateFlattenDecorationPass>(spirv);
			doOptimize<spvtools::CreateUnifyConstantPass>(spirv);
			doOptimize<spvtools::CreateReplaceInvalidOpcodePass>(spirv);
			doOptimize<spvtools::CreateFoldSpecConstantOpAndCompositePass>(spirv);
			doOptimize<spvtools::CreateInsertExtractElimPass>(spirv);
			doOptimize<spvtools::CreateCompactIdsPass>(spirv);
		}
	}
}