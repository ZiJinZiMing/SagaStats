// DamageRuleDefinition.cpp
#include "DamageFramework/DamageRule.h"

UScriptStruct* UDamageRule::GetProducesEffectType() const
{
	if (OperationClass)
	{
		return OperationClass.GetDefaultObject()->GetProducesEffectType();
	}
	return nullptr;
}

TArray<UScriptStruct*> UDamageRule::GetConsumedEffectTypes() const
{
	if (Condition)
	{
		return Condition->GetConsumedEffectTypes();
	}
	return {};
}
