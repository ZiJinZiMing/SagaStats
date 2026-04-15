// DamageRuleFactory.h — 右键菜单 Create → DamageRule 工厂
#pragma once

#include "CoreMinimal.h"
#include "Factories/Factory.h"
#include "DamageRuleFactory.generated.h"

UCLASS(HideCategories = Object)
class UDamageRuleFactory : public UFactory
{
	GENERATED_BODY()

public:
	UDamageRuleFactory();

	// UFactory
	virtual UObject* FactoryCreateNew(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn) override;
	virtual uint32 GetMenuCategories() const override;
	virtual FText GetDisplayName() const override;
};
