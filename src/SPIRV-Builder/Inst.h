#pragma once

#include "Handle.hpp"
#include <vector>
#include <list>
#include <string_view>
#include <optional>

namespace dxil2spirv
{
	namespace spirv
	{
		class Inst
		{
		public:
			Inst(spv::Op opcode) : opcode(opcode) {}
			Inst& setResultType(Handle _id) { id = _id; return *this; }
			Inst& setResultId(Handle _resultid) { resultid = _resultid; return *this; }

			const Inst& dump(std::vector<uint32_t>& output) const;
			const Inst& dump(std::list<Inst>& output) const;

			Handle getResultId() const { return resultid; }
			Handle getResultType() const { return id; }

			template<typename T>
			std::enable_if_t<sizeof(T) == sizeof(uint32_t), Inst&> addOperand(T t)
			{
				uint32_t* pv = reinterpret_cast<uint32_t*>(&t);
				operands.push_back(pv[0]);
				return *this;
			}

			template<typename T>
			std::enable_if_t<sizeof(T) == sizeof(uint64_t), Inst&> addOperand(T t)
			{
				uint32_t* pv = reinterpret_cast<uint32_t*>(&t);
				operands.push_back(pv[0]);
				operands.push_back(pv[1]);
				return *this;
			}

			Inst& addOperand(std::string_view sv);
			Inst& addOperand(const std::vector<Handle>& t);

			template<typename T>
			Inst& addOperand(std::optional<T> t)
			{
				if (t.has_value())
					addOperand(t.value());
				return *this;
			}
		private:
			spv::Op opcode;
			Handle id, resultid;
			std::vector<uint32_t> operands;
		};
	}
}