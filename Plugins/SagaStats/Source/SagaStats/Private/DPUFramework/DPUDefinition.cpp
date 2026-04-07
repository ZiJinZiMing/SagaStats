// DPUDefinition.cpp
#include "DPUFramework/DPUDefinition.h"

UScriptStruct* UDPUDefinition::GetProducesFactType() const
{
	if (LogicClass)
	{
		return LogicClass.GetDefaultObject()->GetProducesFactType();
	}
	return nullptr;
}

TArray<UScriptStruct*> UDPUDefinition::GetConsumedFactTypes() const
{
	if (Condition)
	{
		return Condition->GetConsumedFactTypes();
	}
	return {};
}
