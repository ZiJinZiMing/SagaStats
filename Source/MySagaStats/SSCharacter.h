// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemInterface.h"
#include "GameFramework/Character.h"
#include "SSCharacter.generated.h"

UCLASS()
class MYSAGASTATS_API ASSCharacter : public ACharacter, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	ASSCharacter();

protected:
	virtual void PossessedBy(AController* NewController) override;

	virtual void OnRep_PlayerState() override;


	UFUNCTION(BlueprintNativeEvent)
	void PostAbilitySystemComponentInitialize();
	virtual void PostAbilitySystemComponentInitialize_Implementation();


	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere)
	UAbilitySystemComponent* AbilitySystemComponent;
};
