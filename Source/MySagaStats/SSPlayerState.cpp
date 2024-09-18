// Fill out your copyright notice in the Description page of Project Settings.


#include "SSPlayerState.h"
#include "SSAbilitySystemComponent.h"

ASSPlayerState::ASSPlayerState(const FObjectInitializer& ObjectInitializer): Super(ObjectInitializer)
{
	AbilitySystemComponent = CreateDefaultSubobject<USSAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
	AbilitySystemComponent->SetIsReplicated(true);
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Mixed);

	NetUpdateFrequency = 100.f;
}

UAbilitySystemComponent* ASSPlayerState::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

void ASSPlayerState::PostInitializeComponents()
{
	Super::PostInitializeComponents();
	AbilitySystemComponent->InitAbilityActorInfo(this, this);
}
