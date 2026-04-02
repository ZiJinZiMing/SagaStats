// DamageContext.cpp — v4.4: Fact 模型，内部存储为 TMap<FName, FInstancedStruct>
#include "DPUFramework/DamageContext.h"
#include "UObject/UnrealType.h"
#include "GameplayTagContainer.h"
#include "SagaStatsLog.h"

// ============================================================================
// 事件上下文（FInstancedStruct 自动展开）
// ============================================================================

void UDamageContext::SetEventContext(const FInstancedStruct& InContext)
{
	EventContext = InContext;
	ExpandStructToFields(EventContext);
}

void UDamageContext::ExpandStructToFields(const FInstancedStruct& Struct)
{
	const UScriptStruct* ScriptStruct = Struct.GetScriptStruct();
	const uint8* Memory = Struct.GetMemory();
	if (!ScriptStruct || !Memory) return;

	for (TFieldIterator<FProperty> It(ScriptStruct); It; ++It)
	{
		const FProperty* Prop = *It;
		const FName FieldName = FName(*Prop->GetAuthoredName());

		if (const FBoolProperty* BoolProp = CastField<FBoolProperty>(Prop))
		{
			SetBool(FieldName, BoolProp->GetPropertyValue_InContainer(Memory));
		}
		else if (const FFloatProperty* FloatProp = CastField<FFloatProperty>(Prop))
		{
			SetFloat(FieldName, FloatProp->GetPropertyValue_InContainer(Memory));
		}
		else if (const FDoubleProperty* DoubleProp = CastField<FDoubleProperty>(Prop))
		{
			SetFloat(FieldName, static_cast<float>(DoubleProp->GetPropertyValue_InContainer(Memory)));
		}
		else if (const FIntProperty* IntProp = CastField<FIntProperty>(Prop))
		{
			SetInt(FieldName, IntProp->GetPropertyValue_InContainer(Memory));
		}
		else if (const FNameProperty* NameProp = CastField<FNameProperty>(Prop))
		{
			SetName(FieldName, NameProp->GetPropertyValue_InContainer(Memory));
		}
		else if (const FStructProperty* StructProp = CastField<FStructProperty>(Prop))
		{
			if (StructProp->Struct == TBaseStructure<FVector>::Get())
			{
				const FVector* Vec = StructProp->ContainerPtrToValuePtr<FVector>(Memory);
				SetVector(FieldName, *Vec);
			}
			else if (StructProp->Struct == FGameplayTag::StaticStruct())
			{
				const FGameplayTag* Tag = StructProp->ContainerPtrToValuePtr<FGameplayTag>(Memory);
				SetTag(FieldName, *Tag);
			}
		}
		else if (const FEnumProperty* EnumProp = CastField<FEnumProperty>(Prop))
		{
			// 枚举展开为 Name（枚举值名称）
			const FNumericProperty* UnderlyingProp = EnumProp->GetUnderlyingProperty();
			int64 EnumValue = UnderlyingProp->GetSignedIntPropertyValue(Prop->ContainerPtrToValuePtr<void>(Memory));
			FName EnumName = EnumProp->GetEnum()->GetNameByValue(EnumValue);
			SetName(FieldName, EnumName);
		}

		ContextFieldNames.Add(FieldName);
	}
}

// ============================================================================
// Fact 存储（v4.4 核心 API）
// ============================================================================

void UDamageContext::SetFactGeneric(FName Key, const FInstancedStruct& Value)
{
	Facts.Add(Key, Value);
}

FInstancedStruct UDamageContext::GetFactGeneric(FName Key) const
{
	if (const FInstancedStruct* Found = Facts.Find(Key))
	{
		return *Found;
	}
	return FInstancedStruct();
}

bool UDamageContext::HasFact(FName Key) const
{
	return Facts.Contains(Key);
}

// v4.6: EvaluateFactMethod 和 GetFactFieldAsFloat 已删除
// 领域方法由 ConditionNode 子类直接调用 DC->GetFact<T>(DPUName)

// ExtractFloatFromStruct 保留——被标量兼容接口（GetBool/GetFloat/GetInt）使用
float UDamageContext::ExtractFloatFromStruct(const FInstancedStruct& Struct, FName FieldName)
{
	if (!Struct.IsValid()) return 0.f;

	const UScriptStruct* ScriptStruct = Struct.GetScriptStruct();
	const uint8* Memory = Struct.GetMemory();

	if (FieldName.IsNone())
	{
		if (const FDCFact_Float* F = Struct.GetPtr<FDCFact_Float>()) return F->Value;
		if (const FDCFact_Int* I = Struct.GetPtr<FDCFact_Int>()) return static_cast<float>(I->Value);
		if (const FDCFact_Bool* B = Struct.GetPtr<FDCFact_Bool>()) return B->Value ? 1.f : 0.f;
		return 0.f;
	}

	if (!ScriptStruct || !Memory) return 0.f;

	const FProperty* Prop = ScriptStruct->FindPropertyByName(FieldName);
	if (!Prop) return 0.f;

	if (const FFloatProperty* FloatProp = CastField<FFloatProperty>(Prop))
		return FloatProp->GetPropertyValue_InContainer(Memory);
	if (const FDoubleProperty* DoubleProp = CastField<FDoubleProperty>(Prop))
		return static_cast<float>(DoubleProp->GetPropertyValue_InContainer(Memory));
	if (const FIntProperty* IntProp = CastField<FIntProperty>(Prop))
		return static_cast<float>(IntProp->GetPropertyValue_InContainer(Memory));
	if (const FBoolProperty* BoolProp = CastField<FBoolProperty>(Prop))
		return BoolProp->GetPropertyValue_InContainer(Memory) ? 1.f : 0.f;

	return 0.f;
}

// ============================================================================
// 标量兼容接口（通过包装器 USTRUCT 存入 FInstancedStruct）
// ============================================================================

void UDamageContext::SetBool(FName Key, bool Value)
{
	SetFact<FDCFact_Bool>(Key, FDCFact_Bool{Value});
}

void UDamageContext::SetFloat(FName Key, float Value)
{
	SetFact<FDCFact_Float>(Key, FDCFact_Float{Value});
}

void UDamageContext::SetInt(FName Key, int32 Value)
{
	SetFact<FDCFact_Int>(Key, FDCFact_Int{Value});
}

void UDamageContext::SetName(FName Key, FName Value)
{
	SetFact<FDCFact_Name>(Key, FDCFact_Name{Value});
}

void UDamageContext::SetVector(FName Key, const FVector& Value)
{
	SetFact<FDCFact_Vector>(Key, FDCFact_Vector{Value});
}

void UDamageContext::SetTag(FName Key, FGameplayTag Value)
{
	SetFact<FDCFact_Tag>(Key, FDCFact_Tag{Value});
}

bool UDamageContext::GetBool(FName Key) const
{
	const FInstancedStruct* Found = Facts.Find(Key);
	if (!Found || !Found->IsValid()) return false; // R3

	if (const FDCFact_Bool* B = Found->GetPtr<FDCFact_Bool>()) return B->Value;
	// 非 Bool 包装器：通过 float 转换判断
	return ExtractFloatFromStruct(*Found, NAME_None) != 0.f;
}

float UDamageContext::GetFloat(FName Key) const
{
	const FInstancedStruct* Found = Facts.Find(Key);
	if (!Found || !Found->IsValid()) return 0.f;

	if (const FDCFact_Float* F = Found->GetPtr<FDCFact_Float>()) return F->Value;
	return ExtractFloatFromStruct(*Found, NAME_None);
}

int32 UDamageContext::GetInt(FName Key) const
{
	const FInstancedStruct* Found = Facts.Find(Key);
	if (!Found || !Found->IsValid()) return 0;

	if (const FDCFact_Int* I = Found->GetPtr<FDCFact_Int>()) return I->Value;
	return static_cast<int32>(ExtractFloatFromStruct(*Found, NAME_None));
}

FGameplayTag UDamageContext::GetTag(FName Key) const
{
	const FInstancedStruct* Found = Facts.Find(Key);
	if (!Found || !Found->IsValid()) return FGameplayTag();

	if (const FDCFact_Tag* T = Found->GetPtr<FDCFact_Tag>()) return T->Value;
	return FGameplayTag();
}

// ============================================================================
// 查询 / 生命周期 / 调试
// ============================================================================

bool UDamageContext::Contains(FName Key) const
{
	return Facts.Contains(Key);
}

void UDamageContext::Reset()
{
	Facts.Empty();
	EventContext.Reset();
	ContextFieldNames.Empty();
}

FString UDamageContext::DumpToString() const
{
	FString Result = TEXT("DC Facts:\n");
	for (const auto& Pair : Facts)
	{
		const bool bIsContext = ContextFieldNames.Contains(Pair.Key);
		FString ValueStr;

		if (!Pair.Value.IsValid())
		{
			ValueStr = TEXT("(invalid)");
		}
		else if (const FDCFact_Bool* B = Pair.Value.GetPtr<FDCFact_Bool>())
		{
			ValueStr = B->Value ? TEXT("true") : TEXT("false");
		}
		else if (const FDCFact_Float* F = Pair.Value.GetPtr<FDCFact_Float>())
		{
			ValueStr = FString::SanitizeFloat(F->Value);
		}
		else if (const FDCFact_Int* I = Pair.Value.GetPtr<FDCFact_Int>())
		{
			ValueStr = FString::FromInt(I->Value);
		}
		else if (const FDCFact_Name* N = Pair.Value.GetPtr<FDCFact_Name>())
		{
			ValueStr = N->Value.ToString();
		}
		else if (const FDCFact_Tag* T = Pair.Value.GetPtr<FDCFact_Tag>())
		{
			ValueStr = T->Value.IsValid() ? T->Value.ToString() : TEXT("None");
		}
		else if (const FDCFact_Vector* V = Pair.Value.GetPtr<FDCFact_Vector>())
		{
			ValueStr = V->Value.ToString();
		}
		else
		{
			// 复杂 Fact：显示类型名
			ValueStr = FString::Printf(TEXT("[%s]"), *Pair.Value.GetScriptStruct()->GetName());
		}

		Result += FString::Printf(TEXT("  %s%s = %s\n"),
			bIsContext ? TEXT("[CTX] ") : TEXT("      "),
			*Pair.Key.ToString(),
			*ValueStr);
	}
	return Result;
}
