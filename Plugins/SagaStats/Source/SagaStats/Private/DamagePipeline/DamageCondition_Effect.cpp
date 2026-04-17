/***************************************************************************************************************
* Plugin:       SagaStats
* Author:       Jinming Zhang
* Description:  SagaStats offers modular damage process and meter systems to support adaptable status management
****************************************************************************************************************/

// DamageCondition_Effect.cpp — 基于 Effect 的条件原子实现
#include "DamagePipeline/DamageCondition_Effect.h"
#include "DamagePipeline/DamageContext.h"

bool UDamageCondition_Effect::EvaluateCondition(const UDamageContext* Context) const
{
	FInstancedStruct EffectValue;
	UScriptStruct* Type = GetEffectType();
	if (Context && Type)
	{
		// Context 是 friend 关系，能调 protected GetEffectByType
		EffectValue = Context->GetEffectByType(Type);
	}

	return Evaluate(Context, EffectValue);
}
