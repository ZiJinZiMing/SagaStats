// DamageConditionBlueprintFactory.h — 右键菜单 Create → DamageCondition 蓝图子类
#pragma once

#include "CoreMinimal.h"
#include "Factories/Factory.h"
#include "DamageConditionBlueprintFactory.generated.h"

/**
 * 创建一个继承自 UDamageCondition 的蓝图类。
 * 父类固定为 UDamageCondition，不弹出 class picker（简化流程）。
 */
UCLASS(HideCategories = Object)
class UDamageConditionBlueprintFactory : public UFactory
{
	GENERATED_BODY()

public:
	UDamageConditionBlueprintFactory();

	// UFactory
	virtual UObject* FactoryCreateNew(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn) override;
	virtual uint32 GetMenuCategories() const override;
	virtual FText GetDisplayName() const override;
};
