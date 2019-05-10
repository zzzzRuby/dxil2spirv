#pragma once

#include "Handle.hpp"
#include <set>
#include <vector>
#include <string>
#include <memory>
#include <optional>

namespace dxil2spirv
{
	namespace spirv
	{
		template<typename T>
		using opt_reference = typename std::optional<typename std::reference_wrapper<T>>;

		template<typename T>
		using opt = typename std::optional<T>;

		class Function;
		class Module
		{
		public:
			Module(const Module&) = delete;
			Module(Module&&) = delete;

			void dump(std::vector<uint32_t>& bytecode) const;
			Module(spv::AddressingModel addressModel, spv::MemoryModel memoryModel)
				: addressModel(addressModel), memoryModel(memoryModel), hasEntry(false) {}

			Module& addCapability(spv::Capability c) { capability.insert(c); return *this; }
			Handle nextHandle() { return ++handleGenerator; }

			Handle addTypeFloat(uint32_t bitwidth);
			Handle addTypeInt(uint32_t bitwidth, bool signedness);
			Handle addTypeVoid();
			Handle addTypeVector(Handle compType, uint32_t compCount);
			Handle addTypeArray(Handle compType, Handle compCount);
			Handle addTypeStruct(const std::vector<Handle>& fieldTypes);
			Handle addTypeMatrix(Handle vectorType, uint32_t columnCount);
			Handle addTypeBool();
			Handle addTypeFunction(Handle returnType,
				opt_reference<const std::vector<Handle>> paramTypes = std::nullopt);
			Handle addTypeImage(Handle sampledType, spv::Dim dim,
				uint32_t depth, uint32_t arrayed, uint32_t ms, uint32_t sampled, spv::ImageFormat imageFormat,
				opt<spv::AccessQualifier> accessQualifier = std::nullopt);
			Handle addTypeSampler();
			Handle addTypeSampledImage(Handle imageType);
			Handle addTypeRuntimeArray(Handle baseType);
			Handle addTypePointer(spv::StorageClass storageClass, Handle type);

			template<typename T> Handle addConstant(Handle type, T v);

			void addMemberName(Handle structType, uint32_t idx, std::string_view name);
			void addName(Handle id, std::string_view name);

			std::shared_ptr<Function> createFunction(Handle resultType, spv::FunctionControlMask functionControl,
				Handle functionType);
			void addEntryPoint(std::shared_ptr<Function> entry,
				spv::ExecutionModel executionModel,
				std::string_view entryName,
				const std::vector<Handle>& interfaces);
		private:
			std::set<spv::Capability> capability;
			std::set<std::string> extensions;
			spv::AddressingModel addressModel;
			spv::MemoryModel memoryModel;

			std::vector<uint32_t> entrypoints, types, debugs, annotations;

			bool hasEntry;
			std::vector<std::shared_ptr<Function>> funcs;

			Handle handleGenerator;
		};
	}
}