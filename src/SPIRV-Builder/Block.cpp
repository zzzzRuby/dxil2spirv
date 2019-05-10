#include "Block.h"
#include "Inst.h"
#include "Function.h"

namespace dxil2spirv
{
	namespace spirv
	{
		void Block::setReturn()
		{
			blockEnd = std::make_unique<Inst>(spv::Op::OpReturn);
		}

		Handle Block::nextHadle()
		{
			return _func.nextHandle();
		}

		void Block::setReturn(Handle returnValue)
		{
			blockEnd = std::make_unique<Inst>(spv::Op::OpReturnValue);
			blockEnd->addOperand(returnValue);
		}

		Block::Block(Handle blockHandle, Function& _func)
			: blockHandle(blockHandle), _func(_func)
		{
			Inst(spv::Op::OpLabel)
				.setResultId(blockHandle)
				.dump(bytecode);
		}

		void Block::dump(std::vector<uint32_t>& output)
		{
			output.insert(output.end(), bytecode.begin(), bytecode.end());

			if (!blockEnd) throw std::bad_exception();

			blockEnd->dump(output);
		}
	}
}