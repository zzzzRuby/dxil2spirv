#include "Translator.h"

namespace dxil2spirv
{
	namespace dxil
	{
		static void ConvertSRV(Context& ctx, const hlsl::DxilResource& dxilSRV)
		{

		}

		void ConvertResources(Context& ctx)
		{
			hlsl::DxilModule& dxilModule = ctx.getDxilModule();
			spirv::Module& spvModule = ctx.getSpirvModule();

			size_t srvCount = dxilModule.GetSRVs().size();
			for (size_t i = 0; i < srvCount; i++)
				ConvertSRV(ctx, dxilModule.GetSRV(static_cast<unsigned int>(i)));
		}
	}
}