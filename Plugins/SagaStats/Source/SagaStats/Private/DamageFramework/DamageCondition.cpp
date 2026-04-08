// DamageCondition.cpp — 条件原子实现（v4.8: 子类即域方法）
#include "DamageFramework/DamageCondition.h"
#include "DamageFramework/DamageContext.h"

// ============================================================================
// UDamageCondition
// ============================================================================

bool UDamageCondition::EvaluateCondition(const UDamageContext* Context) const
{
	FInstancedStruct ConsumedFact;
	UScriptStruct* EffectType = GetConsumedEffectType();
	if (Context && EffectType)
	{
		ConsumedFact = Context->GetEffectByType(EffectType);
	}

	return Evaluate(Context, ConsumedFact);
}

TArray<UScriptStruct*> UDamageCondition::GetConsumedEffectTypes() const
{
	UScriptStruct* Type = GetConsumedEffectType();
	if (Type) return {Type};
	return {};
}

FString UDamageCondition::GetDisplayString() const
{
	return GetClass()->GetName();
}

// ============================================================================
// UDamageCondition_ContextCheck
// ============================================================================

bool UDamageCondition_ContextCheck::Evaluate_Implementation(const UDamageContext* Context, const FInstancedStruct& /*ConsumedFact*/) const
{
	if (!Context || ContextKey.IsNone()) return false;
	return Context->Contains(ContextKey) && Context->GetBool(ContextKey);
}

FString UDamageCondition_ContextCheck::GetDisplayString() const
{
	return FString::Printf(TEXT("Ctx:%s"), *ContextKey.ToString());
}
