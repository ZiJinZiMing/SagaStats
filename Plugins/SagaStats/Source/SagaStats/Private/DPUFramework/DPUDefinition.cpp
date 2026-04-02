// DPUDefinition.cpp
#include "DPUFramework/DPUDefinition.h"

TArray<FName> UDPUDefinition::GetConsumedDPUs() const
{
	if (Condition)
	{
		return Condition->GetConsumedDPUs();
	}
	return {};
}
