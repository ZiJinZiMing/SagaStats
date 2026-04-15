// DamageEffectStructFactory.h — 右键菜单 Create → DamageEffect 蓝图结构体
#pragma once

#include "CoreMinimal.h"
#include "Factories/Factory.h"
#include "DamageEffectStructFactory.generated.h"

/**
 * 创建一个 User Defined Struct（蓝图结构体），用作 DamageEffect 类型。
 * 纯 UDS，无特殊约束（方案 X）——只是入口挂到 DamagePipeline 分类下方便找。
 */
UCLASS(HideCategories = Object)
class UDamageEffectStructFactory : public UFactory
{
	GENERATED_BODY()

public:
	UDamageEffectStructFactory();

	// UFactory
	virtual UObject* FactoryCreateNew(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn) override;
	virtual uint32 GetMenuCategories() const override;
	virtual FText GetDisplayName() const override;
};
