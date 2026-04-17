/***************************************************************************************************************
* Plugin:       SagaStats
* Author:       Jinming Zhang
* Description:  SagaStats offers modular damage process and meter systems to support adaptable status management
****************************************************************************************************************/

// DamageCondition_Effect.h — 基于 Effect 的条件原子（贡献 R5 产销依赖）
#pragma once

#include "CoreMinimal.h"
#include "StructUtils/InstancedStruct.h"
#include "DamagePipeline/DamageCondition.h"
#include "DamageCondition_Effect.generated.h"

class UDamageContext;

// ============================================================================
// UDamageCondition_Effect — 基于 Effect 的条件原子
// ============================================================================

/**
 * 基于 Effect 的条件原子。子类绑定一个 EffectType（声明 R5 产销依赖），
 * 框架在 Evaluate 前自动按 EffectType 从 DC 预取对应 Effect 传入。
 *
 * 子类写法（C++）：
 *   UCLASS()
 *   class UDamageCondition_IsLightning : public UDamageCondition_Effect
 *   {
 *       virtual UScriptStruct* GetEffectType() const override { return FSekiroAttackContext::StaticStruct(); }
 *       virtual bool Evaluate_Implementation(const UDamageContext* Context, const FInstancedStruct& InEffect) const override;
 *   };
 *
 * 子类写法（蓝图）：
 *   通过右键菜单 "Damage Condition Effect (Blueprint)" 创建；
 *   在 Class Defaults 中填 EffectType；override Evaluate 事件。
 *
 * 贡献 R5 产销依赖：Pipeline Build 时通过 GetEffectType() 加入依赖图。
 * 在 Graph 节点上：有输入 Pin + EffectType 色块。
 */
UCLASS(Abstract, DefaultToInstanced, EditInlineNew, Blueprintable, CollapseCategories)
class SAGASTATS_API UDamageCondition_Effect : public UDamageCondition
{
	GENERATED_BODY()

public:
	/** 公共入口：按 EffectType 预取 Effect 后调 Evaluate */
	virtual bool EvaluateCondition(const UDamageContext* Context) const override final;

	/**
	 * 子类重写——BlueprintNativeEvent。
	 * @param Context   共享上下文（访问受限：不能读其他 Effect）
	 * @param InEffect  框架按 EffectType 从 DC 预取的 Effect（缺失时为 invalid FInstancedStruct）
	 */
	UFUNCTION(BlueprintNativeEvent, Category = "DamageCondition")
	bool Evaluate(const UDamageContext* Context, const FInstancedStruct& InEffect) const;
	virtual bool Evaluate_Implementation(const UDamageContext* Context, const FInstancedStruct& InEffect) const { return false; }

	/** EffectType 访问器；C++ 子类 override 返回具体类型，蓝图子类通过 Class Defaults 填充。 */
	virtual UScriptStruct* GetEffectType() const override { return EffectType; }

protected:
	/**
	 * 蓝图子类在 Blueprint Class Defaults 中设置；C++ 子类 override GetEffectType()。
	 * 非 CDO 实例上通过 IsClassDefaultContext() + EditConditionHides 完全隐藏，防止在 DamageRule 内误改。
	 */
	UPROPERTY(EditAnywhere, Category = "DamageCondition", meta = (EditCondition = "IsClassDefaultContext", EditConditionHides))
	UScriptStruct* EffectType = nullptr;

private:
	/** EditCondition 驱动函数：CDO/Archetype 返回 true → EffectType 可见可编辑；普通实例返回 false → 隐藏 */
	UFUNCTION()
	bool IsClassDefaultContext() const { return HasAnyFlags(RF_ClassDefaultObject | RF_ArchetypeObject); }
};
