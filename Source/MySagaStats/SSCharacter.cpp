// Fill out your copyright notice in the Description page of Project Settings.


#include "SSCharacter.h"

#include "AbilitySystemComponent.h"
#include "SSPlayerState.h"


// Sets default values
ASSCharacter::ASSCharacter(): AbilitySystemComponent(nullptr)
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

void ASSCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	ASSPlayerState* PS = GetPlayerStateChecked<ASSPlayerState>();
	AbilitySystemComponent = PS->GetAbilitySystemComponent();
	AbilitySystemComponent->InitAbilityActorInfo(PS, this);

	PostAbilitySystemComponentInitialize();
}

void ASSCharacter::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();

	ASSPlayerState* PS = GetPlayerStateChecked<ASSPlayerState>();
	AbilitySystemComponent = PS->GetAbilitySystemComponent();
	AbilitySystemComponent->InitAbilityActorInfo(PS, this);

	PostAbilitySystemComponentInitialize();
}

void ASSCharacter::PostAbilitySystemComponentInitialize_Implementation()
{
}

UAbilitySystemComponent* ASSCharacter::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}
