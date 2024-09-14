/******************************************************************************************
* Plugin:       SagaStats
* Author:       Jinming Zhang
* Description:  SagaStats is a status system that supports fully blueprintable attribute definitions and value calculations.
******************************************************************************************/


#include "SSAbilitySystemComponent.h"


void USSAbilitySystemComponent::OnRep_SpawnedAttributes(const TArray<UAttributeSet*>& PreviousSpawnedAttributes)
{
	Super::OnRep_SpawnedAttributes(PreviousSpawnedAttributes);

	if (IsUsingRegisteredSubObjectList())
	{
		// Find the attributes that got removed
		for (UAttributeSet* PreviousAttributeSet : PreviousSpawnedAttributes)
		{
			if (PreviousAttributeSet)
			{
				const bool bWasRemoved = GetSpawnedAttributes().Find(PreviousAttributeSet) == INDEX_NONE;
				if (bWasRemoved)
				{
					// RemoveReplicatedSubObject(PreviousAttributeSet);
				}
			}
		}

		// Find the attributes that got added
		for (UAttributeSet* NewAttributeSet : GetSpawnedAttributes())
		{
			if (IsValid(NewAttributeSet))
			{
				const bool bIsAdded = PreviousSpawnedAttributes.Find(NewAttributeSet) == INDEX_NONE;
				if (bIsAdded)
				{
					// AddReplicatedSubObject(NewAttributeSet);
				}
			}
		}
	}

}
