// DamageOperationBlueprintFactory.h — 右键菜单 Create → DamageOperation 蓝图子类
#pragma once

#include "CoreMinimal.h"
#include "Factories/Factory.h"
#include "DamageOperationBlueprintFactory.generated.h"

/**
 * 创建一个继承自 UDamageOperationBase 的蓝图类。
 * 父类固定为 UDamageOperationBase，不弹出 class picker。
 */
UCLASS(HideCategories = Object)
class UDamageOperationBlueprintFactory : public UFactory
{
	GENERATED_BODY()

public:
	UDamageOperationBlueprintFactory();

	// UFactory
	virtual UObject* FactoryCreateNew(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn) override;
	virtual uint32 GetMenuCategories() const override;
	virtual FText GetDisplayName() const override;
};
