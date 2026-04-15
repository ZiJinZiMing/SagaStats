// DamageRuleDefinition.cpp
#include "DamageFramework/DamageRule.h"

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
