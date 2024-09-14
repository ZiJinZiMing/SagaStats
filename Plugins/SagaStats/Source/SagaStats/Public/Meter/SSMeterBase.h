/******************************************************************************************
* Plugin:       SagaStats
* Author:       Jinming Zhang
* Description:  SagaStats is a status system that supports fully blueprintable attribute definitions and value calculations.
******************************************************************************************/

#pragma once

#include "CoreMinimal.h"
#include "VisualLogger/VisualLogger.h"
#include "AbilitySystemComponent.h"
#include "SSAttributeSetBlueprintBase.h"
#include "TickableAttributeSetInterface.h"
#include "SSMeterBase.generated.h"

class USSMeterBase;

#define METER_MINIMUM 0

/**
 * 
 */
UCLASS(abstract, meta = (HideInDetailsView))
class SAGASTATS_API USSMeterBase : public USSAttributeSetBlueprintBase, public ITickableAttributeSetInterface
{
	GENERATED_BODY()

public:
	USSMeterBase(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	SS_ATTRIBUTE_ACCESSORS(Current);
	/*Current value of meter*/
	UPROPERTY(EditDefaultsOnly, Category="Meter", ReplicatedUsing=OnRep_Current, meta=(HideFromMOdifiers))
	FSSGameplayClampedAttributeData Current;
	UFUNCTION()
	void OnRep_Current(const FSSGameplayClampedAttributeData& OldValue)
	{
		SS_GAMEPLAYATTRIBUTE_REPNOTIFY(Current, OldValue);
	}

	SS_ATTRIBUTE_ACCESSORS(Maximum);
	/* Maximum of meter*/
	UPROPERTY(EditDefaultsOnly, Category="Meter", ReplicatedUsing=OnRep_Maximum)
	FGameplayAttributeData Maximum;
	UFUNCTION()
	void OnRep_Maximum(const FGameplayAttributeData& OldValue)
	{
		SS_GAMEPLAYATTRIBUTE_REPNOTIFY(Maximum, OldValue);
	}

public:
	static bool GreaterOrNearlyEqual(float A, float B);

	static bool LessOrNearlyEqual(float A, float B);
	
	UFUNCTION(BlueprintPure, Category="Meter")
	bool IsFilled() const;

	UFUNCTION(BlueprintPure, Category="Meter")
	bool IsEmptied() const;

protected:
	//~begin UAttributeSet interface

	/** Called just after any modification happens to an attribute. */
	virtual void PostAttributeChange(const FGameplayAttribute& Attribute, float OldValue, float NewValue) override;

	virtual void PostGameplayEffectExecute(const struct FGameplayEffectModCallbackData& Data) override;

	virtual void PreAttributeBaseChange(const FGameplayAttribute& Attribute, float& NewValue) const override;
	
	//~end UAttributeSet interface

	UFUNCTION(BlueprintPure, Category="Meter")
	float GetPercent() const;
	
	UFUNCTION(BlueprintNativeEvent, Category="Meter")
	void OnEmptied();
	virtual void OnEmptied_Implementation();

	UFUNCTION(BlueprintNativeEvent, Category="Meter")
	void OnFilled();
	virtual void OnFilled_Implementation();

	virtual void OnMaximumChanged(float OldValue, float NewValue);

	virtual void OnCurrentChanged(float OldValue, float NewValue);
	
	//~begin ITickableAttributeSetInterface interface

	/**
	* Does this attribute set need to tick?
	*
	* @return true if this attribute set should currently be ticking, false otherwise.
	*/
	virtual bool ShouldTick() const override;
	/**
	 * Ticks the attribute set by DeltaTime seconds
	 * 
	 * @param DeltaTime Size of the time step in seconds.
	 */
	virtual void Tick(float DeltaTime) override;
	//~end ITickableAttributeSetInterface interface


	//~ Begin UObject interface
	/** Returns properties that are replicated for the lifetime of the actor channel */
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	//~ End UObject interface
};
