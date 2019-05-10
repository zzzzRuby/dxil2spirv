#include "Translator.h"
#include "../SPIRV-Builder/Block.h"
#include "../SPIRV-Builder/Function.h"

namespace dxil2spirv
{
	namespace dxil
	{
		void ConvertFunctions(Context& ctx)
		{
			spirv::Module& spvModule = ctx.getSpirvModule();

			using Handle = spirv::Handle;
			std::vector<Handle> temp;

			Handle voidType = spvModule.addTypeVoid();
			Handle funcType = spvModule.addTypeFunction(voidType);

			std::shared_ptr<spirv::Function> spvFunc =
				spvModule.createFunction(voidType, spv::FunctionControlMask::MaskNone, funcType);
			std::shared_ptr<spirv::Block> spvFuncBlock = spvFunc->getEntryBlock();

			spvFuncBlock->setReturn();

			spvModule.addEntryPoint(spvFunc, spv::ExecutionModel::Vertex, "main", temp);
		}
	}
}