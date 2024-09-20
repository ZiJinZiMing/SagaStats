// Fill out your copyright notice in the Description page of Project Settings.


#include "Meter/Async/AbilityAsync_WaitMeterFilled.h"

#include "SSAbilitySystemComponent.h"

UAbilityAsync_WaitMeterFilled* UAbilityAsync_WaitMeterFilled::WaitMeterFilled(AActor* TargetActor, TSubclassOf<USSMeterBase> MeterClass)
{
	UAbilityAsync_WaitMeterFilled* Async = NewObject<UAbilityAsync_WaitMeterFilled>();
	Async->SetAbilityActor(TargetActor);
	Async->MeterClass = MeterClass;
	return Async;
}

void UAbilityAsync_WaitMeterFilled::Activate()
{
	Super::Activate();
	if(USSAbilitySystemComponent* ASC = Cast<USSAbilitySystemComponent>(GetAbilitySystemComponent()))
	{
		MeterFilledHandle = ASC->GetMeterFilledDelegate(MeterClass).AddUObject(this,&ThisClass::OnMeterFilledCallback);
	}else
	{
		EndAction();
	}
}

void UAbilityAsync_WaitMeterFilled::EndAction()
{
	if(USSAbilitySystemComponent* ASC = Cast<USSAbilitySystemComponent>(GetAbilitySystemComponent()))
	{
		ASC->GetMeterFilledDelegate(MeterClass).Remove(MeterFilledHandle);
	}
	Super::EndAction();
}

void UAbilityAsync_WaitMeterFilled::OnMeterFilledCallback(USSMeterBase* Meter)
{
	OnFilled.Broadcast(Meter);
}
