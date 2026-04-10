// DamagePipelineGraphSchema.cpp
#include "Graph/DamagePipelineGraphSchema.h"

const FPinConnectionResponse UDamagePipelineGraphSchema::CanCreateConnection(const UEdGraphPin* A, const UEdGraphPin* B) const
{
	return FPinConnectionResponse(CONNECT_RESPONSE_DISALLOW, TEXT("Read-only graph"));
}

FLinearColor UDamagePipelineGraphSchema::GetPinTypeColor(const FEdGraphPinType& PinType) const
{
	// DamageEffect Pin 统一用蓝色系
	if (PinType.PinCategory == TEXT("DamageEffect"))
	{
		return FLinearColor(0.42f, 0.56f, 0.75f); // #6c8ebf 蓝
	}
	return Super::GetPinTypeColor(PinType);
}
