/******************************************************************************************
* Plugin:       SagaStats
* Author:       Jinming Zhang
* Description:  SagaStats is a status system that supports fully blueprintable attribute definitions and value calculations.
******************************************************************************************/


#include "Meter/Async/AbilityAsync_WaitAttributeSetAddOrRemove.h"

#include "SSAbilitySystemComponent.h"

UAbilityAsync_WaitAttributeSetAddOrRemove* UAbilityAsync_WaitAttributeSetAddOrRemove::WaitAttributeSetAddOrRemove(AActor* TargetActor, TSubclassOf<UAttributeSet> SetClass)
{
	UAbilityAsync_WaitAttributeSetAddOrRemove* Async = NewObject<UAbilityAsync_WaitAttributeSetAddOrRemove>();
	Async->SetAbilityActor(TargetActor);
	Async->SetClass = SetClass;
	return Async;
}

void UAbilityAsync_WaitAttributeSetAddOrRemove::Activate()
{
	Super::Activate();
	if(USSAbilitySystemComponent* ASC = Cast<USSAbilitySystemComponent>(GetAbilitySystemComponent()))
	{
		AttributeSetAddOrRemoveHandle = ASC->GetAttributeSetAddOrRemoveDelegate(SetClass).AddUObject(this,&ThisClass::OnAttributeSetAddOrRemoveCallback);
	}else
	{
		EndAction();
	}
}

void UAbilityAsync_WaitAttributeSetAddOrRemove::EndAction()
{
	if(USSAbilitySystemComponent* ASC = Cast<USSAbilitySystemComponent>(GetAbilitySystemComponent()))
	{
		ASC->GetAttributeSetAddOrRemoveDelegate(SetClass).Remove(AttributeSetAddOrRemoveHandle);
	}
	Super::EndAction();
}

void UAbilityAsync_WaitAttributeSetAddOrRemove::OnAttributeSetAddOrRemoveCallback(UAttributeSet* AttributeSet, bool IsAdd)
{
	if (IsAdd)
	{
		OnAdd.Broadcast(AttributeSet);
	}
	else
	{
		OnRemove.Broadcast(AttributeSet);
	}
}
