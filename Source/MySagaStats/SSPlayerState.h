// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemInterface.h"
#include "GameFramework/PlayerState.h"
#include "SSPlayerState.generated.h"

class UAbilitySystemComponent;
/**
 * 
 */
UCLASS()
class MYSAGASTATS_API ASSPlayerState : public APlayerState, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:
	ASSPlayerState(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());


	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere)
	UAbilitySystemComponent* AbilitySystemComponent;

protected:
	virtual void PostInitializeComponents() override;
};
