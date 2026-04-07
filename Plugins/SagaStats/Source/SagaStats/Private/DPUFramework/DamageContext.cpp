// DamageContext.cpp — v4.8: 双存储（ContextFacts + DPUFacts）
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
			const FNumericProperty* UnderlyingProp = EnumProp->GetUnderlyingProperty();
			int64 EnumValue = UnderlyingProp->GetSignedIntPropertyValue(Prop->ContainerPtrToValuePtr<void>(Memory));
			FName EnumName = EnumProp->GetEnum()->GetNameByValue(EnumValue);
			SetName(FieldName, EnumName);
		}

		ContextFieldNames.Add(FieldName);
	}
}

// ============================================================================
// DPU Fact 存储（v4.8: UScriptStruct* key）
// ============================================================================

void UDamageContext::SetFactByType(UScriptStruct* FactType, const FInstancedStruct& Value)
{
	if (FactType) DPUFacts.Add(FactType, Value);
}

FInstancedStruct UDamageContext::GetFactByType(UScriptStruct* FactType) const
{
	if (FactType)
	{
		if (const FInstancedStruct* Found = DPUFacts.Find(FactType))
		{
			return *Found;
		}
	}
	return FInstancedStruct();
}

bool UDamageContext::HasFactByType(UScriptStruct* FactType) const
{
	return FactType && DPUFacts.Contains(FactType);
}

// ============================================================================
// 标量兼容接口（事件上下文 ContextFacts）
// ============================================================================

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

void UDamageContext::SetBool(FName Key, bool Value)  { SetContextValue<FDCFact_Bool>(Key, FDCFact_Bool{Value}); }
void UDamageContext::SetFloat(FName Key, float Value) { SetContextValue<FDCFact_Float>(Key, FDCFact_Float{Value}); }
void UDamageContext::SetInt(FName Key, int32 Value)   { SetContextValue<FDCFact_Int>(Key, FDCFact_Int{Value}); }
void UDamageContext::SetName(FName Key, FName Value)  { SetContextValue<FDCFact_Name>(Key, FDCFact_Name{Value}); }
void UDamageContext::SetVector(FName Key, const FVector& Value) { SetContextValue<FDCFact_Vector>(Key, FDCFact_Vector{Value}); }
void UDamageContext::SetTag(FName Key, FGameplayTag Value)      { SetContextValue<FDCFact_Tag>(Key, FDCFact_Tag{Value}); }

bool UDamageContext::GetBool(FName Key) const
{
	const FInstancedStruct* Found = ContextFacts.Find(Key);
	if (!Found || !Found->IsValid()) return false;
	if (const FDCFact_Bool* B = Found->GetPtr<FDCFact_Bool>()) return B->Value;
	return ExtractFloatFromStruct(*Found, NAME_None) != 0.f;
}

float UDamageContext::GetFloat(FName Key) const
{
	const FInstancedStruct* Found = ContextFacts.Find(Key);
	if (!Found || !Found->IsValid()) return 0.f;
	if (const FDCFact_Float* F = Found->GetPtr<FDCFact_Float>()) return F->Value;
	return ExtractFloatFromStruct(*Found, NAME_None);
}

int32 UDamageContext::GetInt(FName Key) const
{
	const FInstancedStruct* Found = ContextFacts.Find(Key);
	if (!Found || !Found->IsValid()) return 0;
	if (const FDCFact_Int* I = Found->GetPtr<FDCFact_Int>()) return I->Value;
	return static_cast<int32>(ExtractFloatFromStruct(*Found, NAME_None));
}

FGameplayTag UDamageContext::GetTag(FName Key) const
{
	const FInstancedStruct* Found = ContextFacts.Find(Key);
	if (!Found || !Found->IsValid()) return FGameplayTag();
	if (const FDCFact_Tag* T = Found->GetPtr<FDCFact_Tag>()) return T->Value;
	return FGameplayTag();
}

// ============================================================================
// 查询 / 生命周期 / 调试
// ============================================================================

bool UDamageContext::Contains(FName Key) const
{
	return ContextFacts.Contains(Key);
}

void UDamageContext::Reset()
{
	ContextFacts.Empty();
	DPUFacts.Empty();
	EventContext.Reset();
	ContextFieldNames.Empty();
}

FString UDamageContext::DumpToString() const
{
	FString Result = TEXT("DC Context:\n");
	for (const auto& Pair : ContextFacts)
	{
		FString ValueStr;
		if (!Pair.Value.IsValid()) { ValueStr = TEXT("(invalid)"); }
		else if (const FDCFact_Bool* B = Pair.Value.GetPtr<FDCFact_Bool>()) { ValueStr = B->Value ? TEXT("true") : TEXT("false"); }
		else if (const FDCFact_Float* F = Pair.Value.GetPtr<FDCFact_Float>()) { ValueStr = FString::SanitizeFloat(F->Value); }
		else if (const FDCFact_Int* I = Pair.Value.GetPtr<FDCFact_Int>()) { ValueStr = FString::FromInt(I->Value); }
		else if (const FDCFact_Name* N = Pair.Value.GetPtr<FDCFact_Name>()) { ValueStr = N->Value.ToString(); }
		else if (const FDCFact_Tag* T = Pair.Value.GetPtr<FDCFact_Tag>()) { ValueStr = T->Value.IsValid() ? T->Value.ToString() : TEXT("None"); }
		else if (const FDCFact_Vector* V = Pair.Value.GetPtr<FDCFact_Vector>()) { ValueStr = V->Value.ToString(); }
		else { ValueStr = FString::Printf(TEXT("[%s]"), *Pair.Value.GetScriptStruct()->GetName()); }

		Result += FString::Printf(TEXT("  %s = %s\n"), *Pair.Key.ToString(), *ValueStr);
	}

	Result += TEXT("DC DPU Facts:\n");
	for (const auto& Pair : DPUFacts)
	{
		FString TypeName = Pair.Key ? Pair.Key->GetName() : TEXT("null");
		Result += FString::Printf(TEXT("  [%s]\n"), *TypeName);
	}
	return Result;
}
