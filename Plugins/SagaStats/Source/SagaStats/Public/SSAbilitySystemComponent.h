/******************************************************************************************
* Plugin:       SagaStats
* Author:       Jinming Zhang
* Description:  SagaStats is a status system that supports fully blueprintable attribute definitions and value calculations.
******************************************************************************************/

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "SSAbilitySystemComponent.generated.h"


UCLASS(ClassGroup=(AbilitySystem), meta=(BlueprintSpawnableComponent))
class SAGASTATS_API USSAbilitySystemComponent : public UAbilitySystemComponent
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category="Attribute")
	UAttributeSet* AddAttributeSet(TSubclassOf<UAttributeSet> SetClass, const UDataTable* DataTable);

	UFUNCTION(BlueprintCallable, Category="Attribute")
	bool RemoveAttributeSet(UAttributeSet* SetClass);

	UFUNCTION(BlueprintCallable, Category="Attribute")
 	bool RemoveAttributeSetByClass(TSubclassOf<UAttributeSet> SetClass);


protected:
	virtual void OnRep_SpawnedAttributes(const TArray<UAttributeSet*>& PreviousSpawnedAttributes) override;
	
};
