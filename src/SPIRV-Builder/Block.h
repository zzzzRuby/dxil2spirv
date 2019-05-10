#pragma once

#include "Handle.hpp"
#include <vector>
#include <memory>

namespace dxil2spirv
{
	namespace spirv
	{
		class Inst;
		class Function;
		class Block
		{
		public:
			void setReturn(Handle returnValue);
			void setReturn();

			Block(const Block&) = delete;
			Block(Block&&) = delete;
		private:
			Handle nextHadle();

			Function& _func;
			Handle blockHandle;

			Block(Handle blockHandle, Function& _func);

			friend class Function;
			void dump(std::vector<uint32_t>& output);

			std::vector<uint32_t> bytecode;

			std::unique_ptr<Inst> blockEnd;
		};
	}
}