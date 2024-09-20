// Fill out your copyright notice in the Description page of Project Settings.


#include "Meter/Async/AbilityAsync_WaitMeterEmptied.h"

#include "SSAbilitySystemComponent.h"

UAbilityAsync_WaitMeterEmptied* UAbilityAsync_WaitMeterEmptied::WaitMeterEmptied(AActor* TargetActor, TSubclassOf<USSMeterBase> MeterClass)
{
	UAbilityAsync_WaitMeterEmptied* Async = NewObject<UAbilityAsync_WaitMeterEmptied>();
	Async->SetAbilityActor(TargetActor);
	Async->MeterClass = MeterClass;
	return Async;
}

void UAbilityAsync_WaitMeterEmptied::Activate()
{
	Super::Activate();
	if(USSAbilitySystemComponent* ASC = Cast<USSAbilitySystemComponent>(GetAbilitySystemComponent()))
	{
		MeterEmptiedHandle = ASC->GetMeterEmptiedDelegate(MeterClass).AddUObject(this,&ThisClass::OnMeterEmptiedCallback);
	}else
	{
		EndAction();
	}
}

void UAbilityAsync_WaitMeterEmptied::EndAction()
{
	if(USSAbilitySystemComponent* ASC = Cast<USSAbilitySystemComponent>(GetAbilitySystemComponent()))
	{
		ASC->GetMeterEmptiedDelegate(MeterClass).Remove(MeterEmptiedHandle);
	}
	Super::EndAction();
}

void UAbilityAsync_WaitMeterEmptied::OnMeterEmptiedCallback(USSMeterBase* Meter)
{
	OnEmptied.Broadcast(Meter);
}
