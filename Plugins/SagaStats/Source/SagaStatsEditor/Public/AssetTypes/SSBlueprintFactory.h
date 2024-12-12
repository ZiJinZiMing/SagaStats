/******************************************************************************************
* Plugin:       SagaStats
* Author:       Jinming Zhang
* Description:  SagaStats is a status system that supports fully blueprintable attribute definitions and value calculations.
******************************************************************************************/

#pragma once

#include "CoreMinimal.h"
#include "Factories/Factory.h"
#include "SSBlueprintFactory.generated.h"

class USagaAttributeSet;

/**
 * Implements a factory for Agile FSM blueprints.
 */
UCLASS(HideCategories = Object, MinimalAPI)
class USSBlueprintFactory : public UFactory
{
	GENERATED_BODY()

public:
	USSBlueprintFactory(const FObjectInitializer& ObjectInitializer);

	//~ Begin UFactory Interface
	virtual bool ConfigureProperties() override;
	virtual UObject* FactoryCreateNew(UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn, FName CallingContext) override;
	virtual UObject* FactoryCreateNew(UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn) override;
	//~ End UFactory Interface	

public:

	/** The type of blueprint that will be created */
	UPROPERTY(EditAnywhere, Category = SSBlueprintFactory)
	TEnumAsByte<enum EBlueprintType> BlueprintType;

	/** The parent class of the created blueprint */
	UPROPERTY(EditAnywhere, Category = SSBlueprintFactory)
	TSubclassOf<USagaAttributeSet> ParentClass;
	
};
