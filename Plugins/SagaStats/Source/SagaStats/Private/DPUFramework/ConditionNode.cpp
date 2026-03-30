// ConditionNode.cpp — 条件树节点实现
#include "DPUFramework/ConditionNode.h"
#include "DPUFramework/DamageContext.h"
#include "DPUFramework/FactMethodRegistry.h"

// ============================================================================
// UConditionNode 基类
// ============================================================================

bool UConditionNode::EvaluateCondition(const UDamageContext* DC) const
{
	bool Result = Evaluate(DC);
	return bReverse ? !Result : Result;
}

// ============================================================================
// UConditionNode_And
// ============================================================================

bool UConditionNode_And::Evaluate(const UDamageContext* DC) const
{
	if (Children.Num() == 0) return false;

	for (const TObjectPtr<UConditionNode>& Child : Children)
	{
		if (!Child) continue;
		if (!Child->EvaluateCondition(DC))
		{
			return false; // 短路：任一 false 即返回
		}
	}
	return true;
}

TArray<FName> UConditionNode_And::GetConsumedFacts() const
{
	TArray<FName> Result;
	for (const TObjectPtr<UConditionNode>& Child : Children)
	{
		if (!Child) continue;
		for (const FName& Key : Child->GetConsumedFacts())
		{
			Result.AddUnique(Key);
		}
	}
	return Result;
}

FString UConditionNode_And::GetDisplayString() const
{
	TArray<FString> Parts;
	for (const TObjectPtr<UConditionNode>& Child : Children)
	{
		if (!Child) continue;
		FString ChildStr = Child->GetDisplayString();
		if (Child->bReverse)
		{
			ChildStr = FString::Printf(TEXT("!(%s)"), *ChildStr);
		}
		Parts.Add(ChildStr);
	}
	FString Inner = FString::Join(Parts, TEXT(" AND "));
	return bReverse ? FString::Printf(TEXT("!(%s)"), *Inner) : Inner;
}

// ============================================================================
// UConditionNode_Or
// ============================================================================

bool UConditionNode_Or::Evaluate(const UDamageContext* DC) const
{
	if (Children.Num() == 0) return false;

	for (const TObjectPtr<UConditionNode>& Child : Children)
	{
		if (!Child) continue;
		if (Child->EvaluateCondition(DC))
		{
			return true; // 短路：任一 true 即返回
		}
	}
	return false;
}

TArray<FName> UConditionNode_Or::GetConsumedFacts() const
{
	TArray<FName> Result;
	for (const TObjectPtr<UConditionNode>& Child : Children)
	{
		if (!Child) continue;
		for (const FName& Key : Child->GetConsumedFacts())
		{
			Result.AddUnique(Key);
		}
	}
	return Result;
}

FString UConditionNode_Or::GetDisplayString() const
{
	TArray<FString> Parts;
	for (const TObjectPtr<UConditionNode>& Child : Children)
	{
		if (!Child) continue;
		FString ChildStr = Child->GetDisplayString();
		if (Child->bReverse)
		{
			ChildStr = FString::Printf(TEXT("!(%s)"), *ChildStr);
		}
		Parts.Add(ChildStr);
	}
	FString Inner = FString::Join(Parts, TEXT(" OR "));
	return bReverse ? FString::Printf(TEXT("!(%s)"), *Inner) : Inner;
}

// ============================================================================
// UConditionNode_FactQuery
// ============================================================================

bool UConditionNode_FactQuery::Evaluate(const UDamageContext* DC) const
{
	if (!DC || FactKey.IsNone()) return false;

	// 调用 DC->EvaluateFactMethod，内部会通过 FactMethodRegistry 分发
	return DC->EvaluateFactMethod(FactKey, MethodName);
}

TArray<FName> UConditionNode_FactQuery::GetConsumedFacts() const
{
	if (FactKey.IsNone()) return {};
	return {FactKey};
}

FString UConditionNode_FactQuery::GetDisplayString() const
{
	if (MethodName.IsNone() || MethodName == NAME_None)
	{
		return FactKey.ToString(); // 信号 Fact
	}
	return FString::Printf(TEXT("%s.%s"), *FactKey.ToString(), *MethodName.ToString());
}

// ============================================================================
// UConditionNode_Compare
// ============================================================================

bool UConditionNode_Compare::Evaluate(const UDamageContext* DC) const
{
	if (!DC || FactKey.IsNone()) return false;

	// 获取字段值
	float FieldValue = DC->GetFactFieldAsFloat(FactKey, FieldName);

	switch (Operator)
	{
	case ECompareOp::Equal:        return FMath::IsNearlyEqual(FieldValue, Value);
	case ECompareOp::NotEqual:     return !FMath::IsNearlyEqual(FieldValue, Value);
	case ECompareOp::Less:         return FieldValue < Value;
	case ECompareOp::LessEqual:    return FieldValue <= Value;
	case ECompareOp::Greater:      return FieldValue > Value;
	case ECompareOp::GreaterEqual: return FieldValue >= Value;
	default:                       return false;
	}
}

TArray<FName> UConditionNode_Compare::GetConsumedFacts() const
{
	if (FactKey.IsNone()) return {};
	return {FactKey};
}

FString UConditionNode_Compare::GetDisplayString() const
{
	FString OpStr;
	switch (Operator)
	{
	case ECompareOp::Equal:        OpStr = TEXT("=="); break;
	case ECompareOp::NotEqual:     OpStr = TEXT("!="); break;
	case ECompareOp::Less:         OpStr = TEXT("<"); break;
	case ECompareOp::LessEqual:    OpStr = TEXT("<="); break;
	case ECompareOp::Greater:      OpStr = TEXT(">"); break;
	case ECompareOp::GreaterEqual: OpStr = TEXT(">="); break;
	}

	FString Left = FieldName.IsNone()
		? FactKey.ToString()
		: FString::Printf(TEXT("%s.%s"), *FactKey.ToString(), *FieldName.ToString());

	return FString::Printf(TEXT("%s %s %s"), *Left, *OpStr, *FString::SanitizeFloat(Value));
}

// ============================================================================
// 编辑器下拉选项（GetOptions meta）
// ============================================================================

// --- FactQuery ---

TArray<FString> UConditionNode_FactQuery::GetFactKeyOptions() const
{
	TArray<FString> Result;
	UFactMethodRegistry* Registry = UFactMethodRegistry::Get();
	if (!Registry) return Result;

	for (const FName& Key : Registry->GetRegisteredFactKeys())
	{
		Result.Add(Key.ToString());
	}
	return Result;
}

TArray<FString> UConditionNode_FactQuery::GetMethodOptions() const
{
	TArray<FString> Result;
	Result.Add(TEXT("None")); // 信号 Fact 检查

	if (FactKey.IsNone()) return Result;

	UFactMethodRegistry* Registry = UFactMethodRegistry::Get();
	if (!Registry) return Result;

	for (const FName& Method : Registry->GetMethodsForFactKey(FactKey))
	{
		Result.Add(Method.ToString());
	}
	return Result;
}

// --- Compare ---

TArray<FString> UConditionNode_Compare::GetFactKeyOptions() const
{
	TArray<FString> Result;
	UFactMethodRegistry* Registry = UFactMethodRegistry::Get();
	if (!Registry) return Result;

	for (const FName& Key : Registry->GetRegisteredFactKeys())
	{
		Result.Add(Key.ToString());
	}
	return Result;
}

// ============================================================================
// PostEditChangeProperty：FactKey 变化时重置下级属性
// ============================================================================

#if WITH_EDITOR
void UConditionNode_FactQuery::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	if (PropertyChangedEvent.GetPropertyName() == GET_MEMBER_NAME_CHECKED(UConditionNode_FactQuery, FactKey))
	{
		// FactKey 变了 → MethodName 可能不再合法 → 重置为 None
		MethodName = NAME_None;
	}
}

void UConditionNode_Compare::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	if (PropertyChangedEvent.GetPropertyName() == GET_MEMBER_NAME_CHECKED(UConditionNode_Compare, FactKey))
	{
		FieldName = NAME_None;
	}
}
#endif

TArray<FString> UConditionNode_Compare::GetFieldOptions() const
{
	TArray<FString> Result;
	Result.Add(TEXT("None")); // 整体求值

	if (FactKey.IsNone()) return Result;

	UFactMethodRegistry* Registry = UFactMethodRegistry::Get();
	if (!Registry) return Result;

	UScriptStruct* Struct = Registry->GetStructForFactKey(FactKey);
	if (!Struct) return Result;

	for (TFieldIterator<FProperty> It(Struct); It; ++It)
	{
		const FProperty* Prop = *It;
		if (CastField<FFloatProperty>(Prop) || CastField<FDoubleProperty>(Prop)
			|| CastField<FIntProperty>(Prop) || CastField<FBoolProperty>(Prop))
		{
			Result.Add(Prop->GetAuthoredName());
		}
	}
	return Result;
}
