#include "Translator.h"
#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/Module.h>

#include <exception>
#include <iostream>
#include <stack>

namespace dxil2spirv
{
	namespace dxil
	{
		static spirv::Handle ProcessField(Context& ctx,
			const hlsl::DxilFieldAnnotation& fieldAnnotation,
			const llvm::Type* llvmFieldType);

		namespace utils
		{
			static const llvm::Type* GetArraySizeStack(const llvm::Type* llvmType, std::stack<uint32_t>& arrayStack)
			{
				const llvm::Type* topType = llvmType;

				while (topType->getTypeID() == llvm::Type::TypeID::ArrayTyID)
				{
					const llvm::ArrayType* arrayType = llvm::dyn_cast<llvm::ArrayType>(topType);
					topType = arrayType->getArrayElementType();

					uint64_t arraySize = arrayType->getArrayNumElements();
					arrayStack.push(static_cast<uint32_t>(arraySize));
				}

				return topType;
			}
		}

		spirv::Handle Context::findStructType(const hlsl::DxilStructAnnotation* structAnnotation)
		{
			if (auto it = structTypeCache.find(structAnnotation); it != structTypeCache.end())
				return std::get<1>(*it);

			const llvm::StructType* structType = structAnnotation->GetStructType();
			std::string structName = structType->getName();

			unsigned int fieldCount = structAnnotation->GetNumFields();
			std::vector<spirv::Handle> fieldTypes(fieldCount);
			for (unsigned int i = 0; i < fieldCount; i++)
			{
				spirv::Handle fieldType = ProcessField(*this,
					structAnnotation->GetFieldAnnotation(i),
					structType->getContainedType(i));
				fieldTypes[i] = fieldType;
			}

			spirv::Handle result = getSpirvModule().addTypeStruct(fieldTypes);

			getSpirvModule().addName(result, structName);
			for (unsigned int i = 0; i < fieldCount; i++)
			{
				getSpirvModule().addMemberName(result, static_cast<uint32_t>(i),
					structAnnotation->GetFieldAnnotation(i).GetFieldName());
			}
			
			structTypeCache[structAnnotation] = result;

			return result;
		}

		spirv::Handle Context::findCompType(hlsl::CompType compType)
		{
			if (auto it = compTypeCache.find(compType.GetKind()); it != compTypeCache.end())
				return std::get<1>(*it);

			spirv::Module& spvModule = getSpirvModule();
			spirv::Handle result;

			switch (compType.GetKind())
			{
			case hlsl::CompType::Kind::SNormF16:
			case hlsl::CompType::Kind::UNormF16:
				result = findCompType(hlsl::CompType::Kind::F16);
				break;
			case hlsl::CompType::Kind::F16:
				result = spvModule.addTypeFloat(16);
				break;
			case hlsl::CompType::Kind::SNormF32:
			case hlsl::CompType::Kind::UNormF32:
				result = findCompType(hlsl::CompType::Kind::F32);
				break;
			case hlsl::CompType::Kind::F32:
				result = spvModule.addTypeFloat(32);
				break;
			case hlsl::CompType::Kind::UNormF64:
			case hlsl::CompType::Kind::SNormF64:
				result = findCompType(hlsl::CompType::Kind::F64);
				break;
			case hlsl::CompType::Kind::F64:
				result = spvModule.addTypeFloat(64);
				break;
			case hlsl::CompType::Kind::I16:
				spvModule.addCapability(spv::Capability::Int16);
				result = spvModule.addTypeInt(16, true);
				break;
			case hlsl::CompType::Kind::I32:
				result = spvModule.addTypeInt(32, true);
				break;
			case hlsl::CompType::Kind::I64:
				result = spvModule.addTypeInt(64, true);
				break;
			case hlsl::CompType::Kind::U16:
				spvModule.addCapability(spv::Capability::Int16);
				result = spvModule.addTypeInt(16, false);
				break;
			case hlsl::CompType::Kind::U32:
				result = spvModule.addTypeInt(32, false);
				break;
			case hlsl::CompType::Kind::U64:
				result = spvModule.addTypeInt(64, false);
				break;
			case hlsl::CompType::Kind::I1:
				result = spvModule.addTypeBool();
				break;
			default:
				throw std::bad_exception();
			}

			compTypeCache[compType.GetKind()] = result;
			return result;
		}

		spirv::Handle Context::findArrayType(spirv::Handle baseType, uint32_t size)
		{
			auto key = std::make_tuple(baseType, size);

			if (auto it = arrayTypeCache.find(key); it != arrayTypeCache.end())
				return std::get<1>(*it);

			spirv::Handle constType = findCompType(hlsl::CompType::getU32());

			spirv::Handle constant = getSpirvModule().addConstant(constType, size);

			spirv::Handle result = getSpirvModule().addTypeArray(baseType, constant);
			arrayTypeCache[key] = result;
			return result;
		}

		spirv::Handle Context::findVectorType(spirv::Handle baseType, uint32_t size)
		{
			auto key = std::make_tuple(baseType, size);

			if (auto it = vectorTypeCache.find(key); it != vectorTypeCache.end())
				return std::get<1>(*it);

			spirv::Handle result = getSpirvModule().addTypeVector(baseType, size);
			vectorTypeCache[key] = result;
			return result;
		}

		spirv::Handle Context::findMatrixType(hlsl::CompType compType, const hlsl::DxilMatrixAnnotation& matrixAnnotation)
		{
			auto key = std::make_tuple(compType.GetKind(), matrixAnnotation.Cols,
				matrixAnnotation.Rows, matrixAnnotation.Orientation);
			if (auto it = matrixTypeCache.find(key); it != matrixTypeCache.end())
				return std::get<1>(*it);

			spirv::Handle baseType = findCompType(compType);
			spirv::Handle vectorType = findVectorType(baseType, matrixAnnotation.Cols);

			spirv::Handle result = getSpirvModule().addTypeMatrix(vectorType, matrixAnnotation.Rows);

			matrixTypeCache[key] = result;
			return result;
		}

		spirv::Handle Context::findVoidType()
		{
			if (!voidTypeCache)
				voidTypeCache = getSpirvModule().addTypeVoid();
			return voidTypeCache;
		}

		static spirv::Handle ProcessField(Context& ctx,
			const hlsl::DxilFieldAnnotation& fieldAnnotation,
			const llvm::Type* llvmFieldType)
		{
			hlsl::CompType compType = fieldAnnotation.GetCompType();
			const hlsl::DxilMatrixAnnotation& matrixAnnotation = fieldAnnotation.GetMatrixAnnotation();
			if (matrixAnnotation.Orientation == hlsl::MatrixOrientation::Undefined)
			{
				std::stack<uint32_t> arrayStack;
				const llvm::Type* topType = utils::GetArraySizeStack(llvmFieldType, arrayStack);

				spirv::Handle arrayBaseType;
				if (topType->getTypeID() == llvm::Type::TypeID::VectorTyID)
				{
					spirv::Handle spvCompType = ctx.findCompType(compType);
					const llvm::VectorType* vectorType = llvm::dyn_cast<llvm::VectorType>(topType);
					arrayBaseType = ctx.findVectorType(spvCompType,
						static_cast<uint32_t>(vectorType->getVectorNumElements()));
				}
				else if (topType->getTypeID() == llvm::Type::TypeID::StructTyID)
				{
					const hlsl::DxilTypeSystem& ts = ctx.getDxilModule().GetTypeSystem();
					const llvm::StructType* structType = llvm::dyn_cast<llvm::StructType>(topType);
					arrayBaseType = ctx.findStructType(ts.GetStructAnnotation(structType));
				}
				else arrayBaseType = ctx.findCompType(compType);

				while (arrayStack.size())
				{
					uint32_t topSize = arrayStack.top();
					arrayBaseType = ctx.findArrayType(arrayBaseType, topSize);
					arrayStack.pop();
				}

				return arrayBaseType;
			}
			else
				return ctx.findMatrixType(compType, matrixAnnotation);
		}

		void ConvertTypes(Context& ctx)
		{
			hlsl::DxilModule& dxilModule = ctx.getDxilModule();
			spirv::Module& spirvModule = ctx.getSpirvModule();

			hlsl::DxilTypeSystem& dxilTypeSystem = dxilModule.GetTypeSystem();

			hlsl::DxilTypeSystem::StructAnnotationMap& dxilStructTypes 
				= dxilTypeSystem.GetStructAnnotationMap();

			for (auto& it : dxilStructTypes)
				ctx.findStructType(std::get<1>(it).get());
		}
	}
}