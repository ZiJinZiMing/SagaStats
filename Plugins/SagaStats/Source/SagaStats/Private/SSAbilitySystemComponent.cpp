/******************************************************************************************
* Plugin:       SagaStats
* Author:       Jinming Zhang
* Description:  SagaStats is a status system that supports fully blueprintable attribute definitions and value calculations.
******************************************************************************************/


#include "SSAbilitySystemComponent.h"
#include "Meter/SSMeterBase.h"


void USSAbilitySystemComponent::RemoveAttributeSet(UAttributeSet* AttributeSet)
{
	RemoveSpawnedAttribute(AttributeSet);
}

void USSAbilitySystemComponent::RemoveAttributeSetByClass(TSubclassOf<UAttributeSet> AttributeSetClass)
{
	if (UAttributeSet* Set = const_cast<UAttributeSet*>(GetAttributeSet(AttributeSetClass)))
	{
		RemoveAttributeSet(Set);
	}
}

FOnAttributeSetAddOrRemoveEvent& USSAbilitySystemComponent::GetAttributeSetAddOrRemoveDelegate(TSubclassOf<UAttributeSet> SetClass)
{
	return AttributeSetAddOrRemoveDelegates.FindOrAdd(SetClass);
}

FOnMeterEmptiedEvent& USSAbilitySystemComponent::GetMeterEmptiedDelegate(TSubclassOf<USSMeterBase> MeterClass)
{
	return MeterEmptiedDelegates.FindOrAdd(MeterClass);
}

FOnMeterFilledEvent& USSAbilitySystemComponent::GetMeterFilledDelegate(TSubclassOf<USSMeterBase> MeterClass)
{
	return MeterFilledDelegates.FindOrAdd(MeterClass);
}

void USSAbilitySystemComponent::AddSpawnedAttribute(UAttributeSet* AttributeSet)
{
	if (!IsValid(AttributeSet))
	{
		return;
	}

	if (GetSpawnedAttributes().Find(AttributeSet) == INDEX_NONE)
	{
		GetAttributeSetAddOrRemoveDelegate(AttributeSet->GetClass()).Broadcast(AttributeSet, true);
	}

	Super::AddSpawnedAttribute(AttributeSet);
}

void USSAbilitySystemComponent::RemoveSpawnedAttribute(UAttributeSet* AttributeSet)
{
	if(GetSpawnedAttributes().Contains(AttributeSet))
	{
		GetAttributeSetAddOrRemoveDelegate(AttributeSet->GetClass()).Broadcast(AttributeSet, false);

	}
	Super::RemoveSpawnedAttribute(AttributeSet);
}

void USSAbilitySystemComponent::RemoveAllSpawnedAttributes()
{
	for (UAttributeSet* AttributeSet : GetSpawnedAttributes())
	{
		GetAttributeSetAddOrRemoveDelegate(AttributeSet->GetClass()).Broadcast(AttributeSet, false);

	}
	Super::RemoveAllSpawnedAttributes();
}

void USSAbilitySystemComponent::OnRep_SpawnedAttributes(const TArray<UAttributeSet*>& PreviousSpawnedAttributes)
{
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
					GetAttributeSetAddOrRemoveDelegate(PreviousAttributeSet->GetClass()).Broadcast(PreviousAttributeSet, false);
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
					GetAttributeSetAddOrRemoveDelegate(NewAttributeSet->GetClass()).Broadcast(NewAttributeSet, true);
				}
			}
		}
	}
	
	Super::OnRep_SpawnedAttributes(PreviousSpawnedAttributes);

}
