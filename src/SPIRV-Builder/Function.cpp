#include "Function.h"
#include "Block.h"
#include "Module.h"
#include "Inst.h"

namespace dxil2spirv
{
	namespace spirv
	{
		std::shared_ptr<Block> Function::createBlock()
		{
			Handle blockHandle = getModule().nextHandle();
			std::shared_ptr<Block> result(new Block(blockHandle, *this));

			blocks.push_back(result);
			return result;
		}

		Handle Function::nextHandle()
		{
			return _module.nextHandle();
		}

		Function::Function(Handle funcHandle, Handle resultType, spv::FunctionControlMask functionControl,
			Handle functionType, Module& _module)
			: funcHandle(funcHandle), _module(_module)
		{
			Inst(spv::Op::OpFunction)
				.setResultId(funcHandle)
				.setResultType(resultType)
				.addOperand(functionControl)
				.addOperand(functionType)
				.dump(bytecode);

			entryBlock = createBlock();
		}

		void Function::dump(std::vector<uint32_t>& output)
		{
			output.insert(output.end(), bytecode.begin(), bytecode.end());
			entryBlock->dump(output);
			for each (std::shared_ptr<Block> block in blocks)
				if (block != entryBlock)
					block->dump(output);

			Inst(spv::Op::OpFunctionEnd)
				.dump(output);
		}
	}
}