/***************************************************************************************************************
* Plugin:       SagaStats
* Author:       Jinming Zhang
* Description:  SagaStats offers modular damage process and meter systems to support adaptable status management
****************************************************************************************************************/

// DamageCondition_Context.h — 基于 Context 的条件原子（不贡献产销依赖）
#pragma once

#include "CoreMinimal.h"
#include "DamagePipeline/DamageCondition.h"
#include "DamageCondition_Context.generated.h"

class UDamageContext;

// ============================================================================
// UDamageCondition_Context — 基于 Context 的条件原子
// ============================================================================

/**
 * 基于 Context 的条件原子。判定只看 UDamageContext 本身（或 Game 侧子类扩展字段），
 * 不声明 EffectType、不贡献 R5 产销依赖、不参与拓扑排序。
 *
 * 子类写法（C++）：
 *   UCLASS()
 *   class UDamageCondition_InTutorial : public UDamageCondition_Context
 *   {
 *       virtual bool Evaluate_Implementation(const UDamageContext* Context) const override
 *       {
 *           const USekiroDamageContext* SekiroCtx = Cast<USekiroDamageContext>(Context);
 *           return SekiroCtx && SekiroCtx->bIsInTutorial;
 *       }
 *   };
 *
 * 子类写法（蓝图）：
 *   通过右键菜单 "Damage Condition Context (Blueprint)" 创建；
 *   override Evaluate 事件。
 *
 * 访问约束（R5 强制）：
 * - ✅ 允许读 Game 侧 UDamageContext 子类的 public 扩展字段（通过 Cast）
 * - ❌ 禁止读 DC 中的任何 DamageEffect —— 签名里根本没有 InEffect 参数；
 *      Context 的 GetEffect<T> 是 protected，子类 Evaluate 里调用会编译失败。
 *
 * 在 Graph 节点上：
 * - 无输入 Pin（不贡献产销）
 * - 无 EffectType 色块（GetEffectType() 返回 nullptr）
 */
UCLASS(Abstract, DefaultToInstanced, EditInlineNew, Blueprintable, CollapseCategories)
class SAGASTATS_API UDamageCondition_Context : public UDamageCondition
{
	GENERATED_BODY()

public:
	/** 公共入口：直接 dispatch 到 Evaluate（不预取任何 Effect） */
	virtual bool EvaluateCondition(const UDamageContext* Context) const override final;

	/**
	 * 子类重写——BlueprintNativeEvent。
	 * @param Context 共享上下文（访问受限：只能读 Game 扩展字段，不能读 Effect）
	 */
	UFUNCTION(BlueprintNativeEvent, Category = "DamageCondition")
	bool Evaluate(const UDamageContext* Context) const;
	virtual bool Evaluate_Implementation(const UDamageContext* Context) const { return false; }

	// 不 override GetEffectType —— 基类默认 nullptr → 不贡献拓扑依赖
};
