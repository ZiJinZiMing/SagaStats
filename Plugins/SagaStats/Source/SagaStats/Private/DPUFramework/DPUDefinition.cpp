// DPUDefinition.cpp
#include "DPUFramework/DPUDefinition.h"

TArray<FName> UDPUDefinition::GetConsumedFacts() const
{
	if (Condition)
	{
		return Condition->GetConsumedFacts();
	}
	return {};
}
