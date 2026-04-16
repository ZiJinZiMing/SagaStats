/***************************************************************************************************************
* Plugin:       SagaStats
* Author:       Claude(@JinmingZhang)
* Description:  SagaStats offers modular damage process and meter systems to support adaptable status management
****************************************************************************************************************/

// DamagePipelineFactory.h — 右键菜单 Create → DamagePipeline 工厂
#pragma once

#include "CoreMinimal.h"
#include "Factories/Factory.h"
#include "DamagePipelineFactory.generated.h"

UCLASS(HideCategories = Object)
class UDamagePipelineFactory : public UFactory
{
	GENERATED_BODY()

public:
	UDamagePipelineFactory();

	// UFactory
	virtual UObject* FactoryCreateNew(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn) override;
	virtual uint32 GetMenuCategories() const override;
	virtual FText GetDisplayName() const override;
};
