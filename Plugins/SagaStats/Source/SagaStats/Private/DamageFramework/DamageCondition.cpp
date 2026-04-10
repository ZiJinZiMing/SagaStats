// DamageCondition.cpp — 条件原子实现（子类即域方法）
#include "DamageFramework/DamageCondition.h"
#include "DamageFramework/DamageContext.h"

// ============================================================================
// UDamageCondition
// ============================================================================

bool UDamageCondition::EvaluateCondition(const UDamageContext* Context) const
{
	FInstancedStruct ConsumedEffect;
	UScriptStruct* EffectType = GetConsumedEffectType();
	if (Context && EffectType)
	{
		ConsumedEffect = Context->GetEffectByType(EffectType);
	}

	return Evaluate(Context, ConsumedEffect);
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
