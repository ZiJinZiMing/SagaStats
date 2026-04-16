/***************************************************************************************************************
* Plugin:       SagaStats
* Author:       Jinming Zhang
* Description:  SagaStats offers modular damage process and meter systems to support adaptable status management
****************************************************************************************************************/

// DamageCondition.cpp — 条件原子实现（子类即域方法）
#include "DamagePipeline/DamageCondition.h"
#include "DamagePipeline/DamageContext.h"

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

