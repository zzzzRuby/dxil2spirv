#pragma once

#include "Handle.hpp"
#include <vector>
#include <memory>

namespace dxil2spirv
{
	namespace spirv
	{
		class Module;
		class Block;
		class Function
		{
		public:
			std::shared_ptr<Block> createBlock();
			std::shared_ptr<Block> getEntryBlock() { return entryBlock; }
			Module& getModule() { return _module; }

			Function(const Function&) = delete;
			Function(Function&&) = delete;
			Handle nextHandle();
		private:

			Function(Handle funcHandle, Handle resultType, spv::FunctionControlMask functionControl,
				Handle functionType, Module& _module);
			void dump(std::vector<uint32_t>& output);

			friend class Module;
			Handle funcHandle;
			Module& _module;

			std::shared_ptr<Block> entryBlock;
			std::vector<std::shared_ptr<Block>> blocks;

			std::vector<uint32_t> bytecode;
		};
	}
}