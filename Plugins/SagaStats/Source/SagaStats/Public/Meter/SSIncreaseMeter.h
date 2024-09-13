/******************************************************************************************
* Plugin:       SagaStats
* Author:       Jinming Zhang
* Description:  SagaStats is a status system that supports fully blueprintable attribute definitions and value calculations.
******************************************************************************************/

#pragma once

#include "CoreMinimal.h"
#include "SSMeterBase.h"
#include "SSIncreaseMeter.generated.h"

/**
 * 
 */
UCLASS()
class SAGASTATS_API USSIncreaseMeter : public USSMeterBase
{
	GENERATED_BODY()

public:
	
	explicit USSIncreaseMeter(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	
	SS_ATTRIBUTE_ACCESSORS(Accumulate);
	UPROPERTY(EditDefaultsOnly, Category="Meter")
	FGameplayAttributeData Accumulate;

	SS_ATTRIBUTE_ACCESSORS(ImpactedAccumulate);
	UPROPERTY(EditDefaultsOnly, Category="Meter", meta=(HideFromMOdifiers))
	FGameplayAttributeData ImpactedAccumulate;
	
	SS_ATTRIBUTE_ACCESSORS(Degeneration);
	UPROPERTY(EditDefaultsOnly, Category="Meter", ReplicatedUsing=OnRep_Degeneration)
	FGameplayAttributeData Degeneration;
	UFUNCTION()
	void OnRep_Degeneration(const FGameplayAttributeData& OldValue)
	{
		SS_GAMEPLAYATTRIBUTE_REPNOTIFY(Degeneration, OldValue);
	}

	SS_ATTRIBUTE_ACCESSORS(DegenerationCooldown);
	UPROPERTY(EditDefaultsOnly, Category="Meter", ReplicatedUsing=OnRep_DegenerationCooldown)
	FGameplayAttributeData DegenerationCooldown;
	UFUNCTION()
	void OnRep_DegenerationCooldown(const FGameplayAttributeData& OldValue)
	{
		SS_GAMEPLAYATTRIBUTE_REPNOTIFY(DegenerationCooldown, OldValue);
	}
	
protected:

	UFUNCTION(BlueprintNativeEvent)
	void OnAccumulate(const FSSAttributeSetExecutionData& Data);
	void OnAccumulate_Implementation(const FSSAttributeSetExecutionData& Data);
	
	virtual void PostGameplayEffectExecute(const struct FGameplayEffectModCallbackData& Data) override;

	
	virtual void Tick(float DeltaTime) override;

	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
	
	bool CanRegen() const;

	UPROPERTY(transient,VisibleInstanceOnly,BlueprintReadOnly, Category="Runtime")
	FTimerHandle DegenerationCooldownTimer;

	UFUNCTION()
	void OnDegenerationCooldownTimerFinish();
	
};
