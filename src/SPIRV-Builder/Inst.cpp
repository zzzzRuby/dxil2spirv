#include "Inst.h"
#include <array>

namespace dxil2spirv
{
	namespace spirv
	{
		const Inst& Inst::dump(std::vector<uint32_t>& output) const
		{
			uint32_t wordCount = static_cast<uint32_t>(operands.size()) + 1;
			if (id) wordCount++;
			if (resultid) wordCount++;

			uint32_t temp =
				((wordCount & 0xffff) << 16) |
				(static_cast<uint32_t>(opcode) & 0xffff);

			output.push_back(temp);

			if (id) output.push_back(id);
			if (resultid) output.push_back(resultid);

			output.insert(output.end(), operands.begin(), operands.end());

			return *this;
		}

		const Inst& Inst::dump(std::list<Inst>& output) const
		{
			output.push_back(*this);
			return *this;
		}

		Inst& Inst::addOperand(std::string_view sv)
		{
			std::array<char, 4> temp;
			for (size_t i = 0; i < sv.size(); i++)
			{
				size_t charCount = i % 4;

				temp[charCount] = sv[i];

				if (charCount == 3)
				{
					addOperand(temp);
					temp[0] = temp[1] = temp[2] = temp[3] = 0;
					charCount = 0;
				}
			}

			temp[sv.size() % 4] = '\0';
			return addOperand(temp);
		}

		Inst& Inst::addOperand(const std::vector<Handle>& t)
		{
			for (Handle v : t)
				addOperand(v);
			return *this;
		}
	}
}