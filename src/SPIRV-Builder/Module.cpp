#include "Module.h"
#include "Function.h"
#include "Inst.h"
#include <algorithm>

namespace dxil2spirv
{
	namespace spirv
	{
		const Handle Handle::Null;

		void Module::dump(std::vector<uint32_t>& bytecode) const
		{
			Handle idBound = handleGenerator;
			bytecode.push_back(static_cast<uint32_t>(spv::MagicNumber));
			bytecode.push_back(static_cast<uint32_t>(spv::Version));
			bytecode.push_back(0u);
			bytecode.push_back(++idBound);
			bytecode.push_back(0u);

			for each (spv::Capability c in capability)
			{
				Inst(spv::Op::OpCapability)
					.addOperand(c)
					.dump(bytecode);
			}

			Inst(spv::Op::OpMemoryModel)
				.addOperand(addressModel)
				.addOperand(memoryModel)
				.dump(bytecode);

			bytecode.insert(bytecode.end(), entrypoints.begin(), entrypoints.end());
			bytecode.insert(bytecode.end(), annotations.begin(), annotations.end());
			bytecode.insert(bytecode.end(), debugs.begin(), debugs.end());
			bytecode.insert(bytecode.end(), types.begin(), types.end());

			if (!hasEntry) throw std::bad_exception();

			for each (std::shared_ptr<Function> func in funcs)
				func->dump(bytecode);
		}

		Handle Module::addTypeFloat(uint32_t bitwidth)
		{
			return Inst(spv::Op::OpTypeFloat)
				.setResultId(nextHandle())
				.addOperand(bitwidth)
				.dump(types)
				.getResultId();
		}

		Handle Module::addTypeInt(uint32_t bitwidth, bool signedness)
		{
			return Inst(spv::Op::OpTypeInt)
				.setResultId(nextHandle())
				.addOperand(bitwidth)
				.addOperand(signedness ? 1u : 0u)
				.dump(types)
				.getResultId();
		}

		Handle Module::addTypeVoid()
		{
			return Inst(spv::Op::OpTypeVoid)
				.setResultId(nextHandle())
				.dump(types)
				.getResultId();
		}

		Handle Module::addTypeVector(Handle compType, uint32_t compCount)
		{
			return Inst(spv::Op::OpTypeVector)
				.setResultId(nextHandle())
				.addOperand(compType)
				.addOperand(compCount)
				.dump(types)
				.getResultId();
		}

		Handle Module::addTypeArray(Handle compType, Handle compCount)
		{
			return Inst(spv::Op::OpTypeArray)
				.setResultId(nextHandle())
				.addOperand(compType)
				.addOperand(compCount)
				.dump(types)
				.getResultId();
		}

		Handle Module::addTypeStruct(const std::vector<Handle>& fieldTypes)
		{
			return Inst(spv::Op::OpTypeStruct)
				.setResultId(nextHandle())
				.addOperand(fieldTypes)
				.dump(types)
				.getResultId();
		}

		Handle Module::addTypeMatrix(Handle vectorType, uint32_t columnCount)
		{
			return Inst(spv::Op::OpTypeMatrix)
				.setResultId(nextHandle())
				.addOperand(vectorType)
				.addOperand(columnCount)
				.dump(types)
				.getResultId();
		}

		Handle Module::addTypeBool()
		{
			return Inst(spv::Op::OpTypeBool)
				.setResultId(nextHandle())
				.dump(types)
				.getResultId();
		}

		Handle Module::addTypeFunction(Handle returnType,
			opt_reference<const std::vector<Handle>> paramTypes)
		{
			return Inst(spv::Op::OpTypeFunction)
				.setResultId(nextHandle())
				.addOperand(returnType)
				.addOperand(paramTypes)
				.dump(types)
				.getResultId();
		}

		Handle Module::addTypeImage(Handle sampledType, spv::Dim dim, 
			uint32_t depth, uint32_t arrayed, uint32_t ms, uint32_t sampled, spv::ImageFormat imageFormat, 
			opt<spv::AccessQualifier> accessQualifier)
		{
			return Inst(spv::Op::OpTypeImage)
				.setResultId(nextHandle())
				.addOperand(sampledType)
				.addOperand(dim)
				.addOperand(depth)
				.addOperand(arrayed)
				.addOperand(ms)
				.addOperand(sampled)
				.addOperand(imageFormat)
				.addOperand(accessQualifier)
				.dump(types)
				.getResultId();
		}

		Handle Module::addTypeSampler()
		{
			return Inst(spv::Op::OpTypeSampler)
				.setResultId(nextHandle())
				.dump(types)
				.getResultId();
		}

		Handle Module::addTypeSampledImage(Handle imageType)
		{
			return Inst(spv::Op::OpTypeSampler)
				.setResultId(nextHandle())
				.dump(types)
				.getResultId();
		}

		template<> Handle Module::addConstant<uint32_t>(Handle u32type, uint32_t v)
		{
			return Inst(spv::Op::OpConstant)
				.setResultId(nextHandle())
				.setResultType(u32type)
				.addOperand(v)
				.dump(types)
				.getResultId();
		}

		template<> Handle Module::addConstant<bool>(Handle booltype, bool v)
		{
			return Inst(v ? spv::Op::OpConstantTrue : spv::Op::OpConstantFalse)
				.setResultId(nextHandle())
				.setResultType(booltype)
				.dump(types)
				.getResultId();
		}

		void Module::addMemberName(Handle structType, uint32_t idx, std::string_view name)
		{
			Inst(spv::Op::OpMemberName)
				.addOperand(structType)
				.addOperand(idx)
				.addOperand(name)
				.dump(debugs);
		}

		void Module::addName(Handle id, std::string_view name)
		{
			Inst(spv::Op::OpName)
				.addOperand(id)
				.addOperand(name)
				.dump(debugs);
		}

		std::shared_ptr<Function> Module::createFunction(Handle resultType,
			spv::FunctionControlMask functionControl, Handle functionType)
		{
			Handle funcHandle = nextHandle();

			std::shared_ptr<Function> result(
				new Function(funcHandle, resultType, functionControl, functionType, *this));

			funcs.push_back(result);
			return result;
		}

		void Module::addEntryPoint(std::shared_ptr<Function> entry,
			spv::ExecutionModel executionModel,
			std::string_view entryName,
			const std::vector<Handle>& interfaces)
		{
			for each (std::shared_ptr<Function> f in funcs)
			{
				if (entry == f)
				{
					hasEntry = true;

					Inst(spv::Op::OpEntryPoint)
						.addOperand(executionModel)
						.addOperand(entry->funcHandle)
						.addOperand(entryName)
						.addOperand(interfaces)
						.dump(entrypoints);

					return;
				}
			}
			throw std::bad_exception();
		}
	}
}