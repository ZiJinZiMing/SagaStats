// DPUCondition.cpp — 条件原子实现（v4.8: 子类即域方法）
#include "DPUFramework/DPUCondition.h"
#include "DPUFramework/DamageContext.h"

// ============================================================================
// UDPUCondition
// ============================================================================

bool UDPUCondition::EvaluateCondition(const UDamageContext* DC) const
{
	FInstancedStruct ConsumedFact;
	UScriptStruct* FactType = GetConsumedFactType();
	if (DC && FactType)
	{
		ConsumedFact = DC->GetFactByType(FactType);
	}

	return Evaluate(DC, ConsumedFact);
}

TArray<UScriptStruct*> UDPUCondition::GetConsumedFactTypes() const
{
	UScriptStruct* Type = GetConsumedFactType();
	if (Type) return {Type};
	return {};
}

FString UDPUCondition::GetDisplayString() const
{
	return GetClass()->GetName();
}

// ============================================================================
// UDPUCondition_ContextCheck
// ============================================================================

bool UDPUCondition_ContextCheck::Evaluate_Implementation(const UDamageContext* DC, const FInstancedStruct& /*ConsumedFact*/) const
{
	if (!DC || ContextKey.IsNone()) return false;
	return DC->Contains(ContextKey) && DC->GetBool(ContextKey);
}

FString UDPUCondition_ContextCheck::GetDisplayString() const
{
	return FString::Printf(TEXT("Ctx:%s"), *ContextKey.ToString());
}
