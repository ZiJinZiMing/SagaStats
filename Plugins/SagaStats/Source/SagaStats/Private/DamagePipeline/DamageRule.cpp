/***************************************************************************************************************
* Plugin:       SagaStats
* Author:       Jinming Zhang
* Description:  SagaStats offers modular damage process and meter systems to support adaptable status management
****************************************************************************************************************/

// DamageRule.cpp
#include "DamagePipeline/DamageRule.h"

UScriptStruct* UDamageRule::GetProducesEffectType() const
{
	if (OperationClass)
	{
		return OperationClass.GetDefaultObject()->GetEffectType();
	}
	return nullptr;
}

TArray<UScriptStruct*> UDamageRule::GetConsumedEffectTypes() const
{
	if (Condition)
	{
		return Condition->GetDependencyEffectTypes();
	}
	return {};
}
