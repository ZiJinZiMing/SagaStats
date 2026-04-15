// DamageCondition.cpp — 条件原子实现（子类即域方法）
#include "DamageFramework/DamageCondition.h"
#include "DamageFramework/DamageContext.h"

// ============================================================================
// UDamageCondition
// ============================================================================

bool UDamageCondition::EvaluateCondition(const UDamageContext* Context) const
{
	FInstancedStruct EffectValue;
	UScriptStruct* Type = GetEffectType();
	if (Context && Type)
	{
		EffectValue = Context->GetEffectByType(Type);
	}

	return Evaluate(Context, EffectValue);
}

