/******************************************************************************************
* Plugin:       SagaStats
* Author:       Jinming Zhang
* Description:  SagaStats is a status system that supports fully blueprintable attribute definitions and value calculations.
******************************************************************************************/

#pragma once

#include "CoreMinimal.h"
#include "SSMeterBase.h"
#include "SSDecreaseMeter.generated.h"


/*State of Meter*/
UENUM(BlueprintType)
enum class EMeterState : uint8
{
	Normal,
	Lock,
	Reset,
};

DECLARE_ENUM_TO_STRING(EMeterState);


/**
 * 
 */
UCLASS()
class SAGASTATS_API USSDecreaseMeter : public USSMeterBase
{
	GENERATED_BODY()

public:
	explicit USSDecreaseMeter(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());


	SS_ATTRIBUTE_ACCESSORS(MinimumClamp)
	/* The minimum clamp value for Current*/
	UPROPERTY(EditDefaultsOnly, Category="Meter", ReplicatedUsing=OnRep_MinimumClamp)
	FGameplayAttributeData MinimumClamp;
	UFUNCTION()
	void OnRep_MinimumClamp(const FGameplayAttributeData& OldValue)
	{
		SS_GAMEPLAYATTRIBUTE_REPNOTIFY(MinimumClamp, OldValue);
	}

	SS_ATTRIBUTE_ACCESSORS(Regeneration)
	/* The minimum clamp value for Current*/
	UPROPERTY(EditDefaultsOnly, Category="Meter", ReplicatedUsing=OnRep_Regeneration)
	FGameplayAttributeData Regeneration;
	UFUNCTION()
	void OnRep_Regeneration(const FGameplayAttributeData& OldValue)
	{
		SS_GAMEPLAYATTRIBUTE_REPNOTIFY(Regeneration, OldValue);
	}

	SS_ATTRIBUTE_ACCESSORS(RegenerationCooldown)
	/* The minimum clamp value for Current*/
	UPROPERTY(EditDefaultsOnly, Category="Meter", ReplicatedUsing=OnRep_RegenerationCooldown)
	FGameplayAttributeData RegenerationCooldown;
	UFUNCTION()
	void OnRep_RegenerationCooldown(const FGameplayAttributeData& OldValue)
	{
		SS_GAMEPLAYATTRIBUTE_REPNOTIFY(RegenerationCooldown, OldValue);
	}
	
	SS_ATTRIBUTE_ACCESSORS(LockDuration)
	/* Duration of lockdown state
	 * >0, keep lockdown state in seconds
	 * =0, keep lockdown state a frame
	 * <0, always keep lockdown state 
	 */
	UPROPERTY(EditDefaultsOnly, Category="Meter", ReplicatedUsing=OnRep_LockDuration)
	FGameplayAttributeData LockDuration;
	UFUNCTION()
	void OnRep_LockDuration(const FGameplayAttributeData& OldValue)
	{
		SS_GAMEPLAYATTRIBUTE_REPNOTIFY(LockDuration, OldValue);
	}


	SS_ATTRIBUTE_ACCESSORS(ResetRate);
	/* Recovery rate in resetting state
	 * >0, recovery rate in reset state
	 * <=0, reset immediately
	 */
	UPROPERTY(EditDefaultsOnly, Category="Meter", ReplicatedUsing=OnRep_ResetRate)
	FGameplayAttributeData ResetRate;
	UFUNCTION()
	void OnRep_ResetRate(const FGameplayAttributeData& OldValue)
	{
		SS_GAMEPLAYATTRIBUTE_REPNOTIFY(ResetRate, OldValue);
	}


	SS_ATTRIBUTE_ACCESSORS(ImmuneThreshold);
	/* Immune in resetting state
	 * >0, maintain immune until the threshold reached
	 * =0(default), no immune in recovery state
	 * <0, maintain immune throughout the resetting state
	 */
	UPROPERTY(EditDefaultsOnly, Category="Meter", ReplicatedUsing=OnRep_ImmuneThreshold)
	FGameplayAttributeData ImmuneThreshold;
	UFUNCTION()
	void OnRep_ImmuneThreshold(const FGameplayAttributeData& OldValue)
	{
		SS_GAMEPLAYATTRIBUTE_REPNOTIFY(ImmuneThreshold, OldValue);
	}


	/*State of meter*/
	UPROPERTY(ReplicatedUsing=OnRep_MeterState, Transient, VisibleInstanceOnly, BlueprintReadOnly, Category="Runtime")
	EMeterState MeterState;

protected:
	UFUNCTION()
	void OnRep_MeterState(const EMeterState& OldValue);

	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
	
};
