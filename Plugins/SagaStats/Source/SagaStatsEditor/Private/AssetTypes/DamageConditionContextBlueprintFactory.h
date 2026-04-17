/***************************************************************************************************************
* Plugin:       SagaStats
* Author:       Claude(@JinmingZhang)
* Description:  SagaStats offers modular damage process and meter systems to support adaptable status management
****************************************************************************************************************/

// DamageConditionContextBlueprintFactory.h — 右键菜单 Create → DamageCondition_Context 蓝图子类
#pragma once

#include "CoreMinimal.h"
#include "Factories/Factory.h"
#include "DamageConditionContextBlueprintFactory.generated.h"

/**
 * 创建一个继承自 UDamageCondition_Context 的蓝图类。
 * Context 版：不声明 EffectType，不贡献 R5 产销依赖，只能看 DamageContext 本身及其 Game 扩展字段。
 * 父类固定为 UDamageCondition_Context，不弹出 class picker。
 */
UCLASS(HideCategories = Object)
class UDamageConditionContextBlueprintFactory : public UFactory
{
	GENERATED_BODY()

public:
	UDamageConditionContextBlueprintFactory();

	// UFactory
	virtual UObject* FactoryCreateNew(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn) override;
	virtual uint32 GetMenuCategories() const override;
	virtual FText GetDisplayName() const override;
};
