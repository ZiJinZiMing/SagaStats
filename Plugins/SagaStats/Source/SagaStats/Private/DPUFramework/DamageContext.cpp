// DamageContext.cpp
#include "DPUFramework/DamageContext.h"
#include "UObject/UnrealType.h"
#include "GameplayTagContainer.h"

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
	if (!ScriptStruct || !Memory)
	{
		return;
	}

	for (TFieldIterator<FProperty> It(ScriptStruct); It; ++It)
	{
		const FProperty* Prop = *It;
		const FName FieldName = Prop->GetFName();

		if (const FBoolProperty* BoolProp = CastField<FBoolProperty>(Prop))
		{
			SetBool(FieldName, BoolProp->GetPropertyValue_InContainer(Memory));
			ContextFieldNames.Add(FieldName);
		}
		else if (const FFloatProperty* FloatProp = CastField<FFloatProperty>(Prop))
		{
			SetFloat(FieldName, FloatProp->GetPropertyValue_InContainer(Memory));
			ContextFieldNames.Add(FieldName);
		}
		else if (const FDoubleProperty* DoubleProp = CastField<FDoubleProperty>(Prop))
		{
			SetFloat(FieldName, static_cast<float>(DoubleProp->GetPropertyValue_InContainer(Memory)));
			ContextFieldNames.Add(FieldName);
		}
		else if (const FIntProperty* IntProp = CastField<FIntProperty>(Prop))
		{
			SetInt(FieldName, IntProp->GetPropertyValue_InContainer(Memory));
			ContextFieldNames.Add(FieldName);
		}
		else if (const FNameProperty* NameProp = CastField<FNameProperty>(Prop))
		{
			SetName(FieldName, NameProp->GetPropertyValue_InContainer(Memory));
			ContextFieldNames.Add(FieldName);
		}
		else if (const FStructProperty* StructProp = CastField<FStructProperty>(Prop))
		{
			// FVector
			if (StructProp->Struct == TBaseStructure<FVector>::Get())
			{
				const FVector* Vec = StructProp->ContainerPtrToValuePtr<FVector>(Memory);
				SetVector(FieldName, *Vec);
				ContextFieldNames.Add(FieldName);
			}
			// FGameplayTag
			else if (StructProp->Struct == FGameplayTag::StaticStruct())
			{
				const FGameplayTag* Tag = StructProp->ContainerPtrToValuePtr<FGameplayTag>(Memory);
				SetTag(FieldName, *Tag);
				ContextFieldNames.Add(FieldName);
			}
		}
	}
}

// ============================================================================
// DPU 产出字段 — 写入
// ============================================================================

void UDamageContext::Set(FName Key, const FDCValue& Value)
{
	Fields.Add(Key, Value);
}

void UDamageContext::SetBool(FName Key, bool Value)
{
	Fields.Add(Key, FDCValue::FromBool(Value));
}

void UDamageContext::SetFloat(FName Key, float Value)
{
	Fields.Add(Key, FDCValue::FromFloat(Value));
}

void UDamageContext::SetInt(FName Key, int32 Value)
{
	Fields.Add(Key, FDCValue::FromInt(Value));
}

void UDamageContext::SetName(FName Key, FName Value)
{
	Fields.Add(Key, FDCValue::FromName(Value));
}

void UDamageContext::SetVector(FName Key, const FVector& Value)
{
	Fields.Add(Key, FDCValue::FromVector(Value));
}

void UDamageContext::SetTag(FName Key, FGameplayTag Value)
{
	Fields.Add(Key, FDCValue::FromTag(Value));
}

// ============================================================================
// DPU 产出字段 — 读取（R3: 缺失字段返回 None）
// ============================================================================

FDCValue UDamageContext::Get(FName Key) const
{
	if (const FDCValue* Found = Fields.Find(Key))
	{
		return *Found;
	}
	return FDCValue(); // None 类型 — AsBool() == false (R3)
}

bool UDamageContext::GetBool(FName Key) const
{
	return Get(Key).AsBool();
}

float UDamageContext::GetFloat(FName Key) const
{
	return Get(Key).AsFloat();
}

int32 UDamageContext::GetInt(FName Key) const
{
	return Get(Key).AsInt();
}

FGameplayTag UDamageContext::GetTag(FName Key) const
{
	return Get(Key).AsTag();
}

// ============================================================================
// 查询 / 生命周期 / 调试
// ============================================================================

bool UDamageContext::Contains(FName Key) const
{
	return Fields.Contains(Key);
}

void UDamageContext::Reset()
{
	Fields.Empty();
	EventContext.Reset();
	ContextFieldNames.Empty();
}

FString UDamageContext::DumpToString() const
{
	FString Result = TEXT("DC Fields:\n");
	for (const auto& Pair : Fields)
	{
		const bool bIsContext = ContextFieldNames.Contains(Pair.Key);
		Result += FString::Printf(TEXT("  %s%s = %s\n"),
			bIsContext ? TEXT("[CTX] ") : TEXT("      "),
			*Pair.Key.ToString(),
			*Pair.Value.ToString());
	}
	return Result;
}
