// DPUDefinition.cpp
#include "DPUFramework/DPUDefinition.h"

TArray<FName> UDPUDefinition::GetConsumedFields() const
{
	if (Condition)
	{
		return Condition->GetConsumedFields();
	}
	return {};
}
