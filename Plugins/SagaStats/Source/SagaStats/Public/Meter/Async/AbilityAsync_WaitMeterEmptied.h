// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/Async/AbilityAsync.h"
#include "AbilityAsync_WaitMeterEmptied.generated.h"


class USSMeterBase;
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnWaitMeterEmptiedEvent, USSMeterBase*, Meter);

/**
 * 
 */
UCLASS()
class SAGASTATS_API UAbilityAsync_WaitMeterEmptied : public UAbilityAsync
{
	GENERATED_BODY()

public:

	UPROPERTY(BlueprintAssignable)
	FOnWaitMeterEmptiedEvent OnEmptied;

	UFUNCTION(BlueprintCallable, Category = "Ability|Async", meta = (DefaultToSelf = "TargetActor", BlueprintInternalUseOnly = "TRUE"))
	static UAbilityAsync_WaitMeterEmptied* WaitMeterEmptied(AActor* TargetActor, TSubclassOf<USSMeterBase> MeterClass);

protected:
	virtual void Activate() override;
	
	virtual void EndAction() override;
	
private:
	FDelegateHandle MeterEmptiedHandle;

	UPROPERTY()
	TSubclassOf<USSMeterBase> MeterClass;

	UFUNCTION()
	void OnMeterEmptiedCallback(USSMeterBase* Meter);
	
};

