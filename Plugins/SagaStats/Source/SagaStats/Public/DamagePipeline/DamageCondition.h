/***************************************************************************************************************
* Plugin:       SagaStats
* Author:       Jinming Zhang
* Description:  SagaStats offers modular damage process and meter systems to support adaptable status management
****************************************************************************************************************/

// DamageCondition.h — 条件原子抽象基类（两种具体形态：_Effect / _Context）
#pragma once

#include "CoreMinimal.h"
#include "DamageCondition.generated.h"

class UDamageContext;

// ============================================================================
// UDamageCondition — 条件原子抽象基类
// ============================================================================

/**
 * UDamageCondition 是条件原子的抽象基类。本类**不直接继承**——应继承两个具体子类之一：
 *
 * - UDamageCondition_Effect —— 基于 Effect 的判定
 *   - 绑定一个 EffectType（声明 R5 产销依赖）
 *   - Evaluate 签名：(Context, InEffect) —— 框架按 EffectType 预取 Effect 传入
 *   - 贡献拓扑排序依赖；在 Graph 节点有输入 Pin + EffectType 色块
 *
 * - UDamageCondition_Context —— 基于 Context 的判定
 *   - 无 EffectType（不声明 R5 依赖）
 *   - Evaluate 签名：(Context) —— 只拿 Context，不传 Effect
 *   - 不贡献拓扑排序；在 Graph 节点无输入 Pin、无色块
 *   - 允许读 Game 侧 UDamageContext 子类的扩展字段（Cast<SekiroContext>->bInTutorial 等）
 *   - **不允许**读其他 Effect —— 签名里根本没有 InEffect 参数；Context 的 GetEffect 是 protected
 *
 * 选择指南：
 * - 判定依赖某条 Rule 产出的 Effect → _Effect 子类
 * - 判定只看 Game 全局状态（tutorial 阶段、boss 阶段、调试开关等）→ _Context 子类
 * - 需要同时看 Effect + Game 扩展字段 → _Effect 子类（Evaluate 里通过 Cast 访问 Context 扩展）
 *
 * 子类即域方法：每个子类对应一个领域判定（如 UDamageCondition_IsGuard / UDamageCondition_InTutorial）。
 */
UCLASS(Abstract, DefaultToInstanced, EditInlineNew, Blueprintable, CollapseCategories)
class SAGASTATS_API UDamageCondition : public UObject
{
	GENERATED_BODY()

public:
	/**
	 * 公共入口——由 UDamagePredicate_Single 调用。
	 * 两个具体子类各自实现：
	 *   _Effect 版：按 EffectType 从 DC 预取 InEffect → 调 Evaluate(Context, InEffect)
	 *   _Context 版：直接调 Evaluate(Context)
	 */
	virtual bool EvaluateCondition(const UDamageContext* Context) const PURE_VIRTUAL(UDamageCondition::EvaluateCondition, return false;);

	/**
	 * 返回本 Condition 所依赖的 EffectType（R5 产销依赖声明）。
	 * - _Effect 子类 override 返回具体 UScriptStruct*
	 * - _Context 子类**不 override**——基类默认返回 nullptr → 不贡献拓扑依赖
	 */
	virtual UScriptStruct* GetEffectType() const { return nullptr; }

	/** 显示字符串（Graph 节点 / Tooltip 用；蓝图可 override） */
	UFUNCTION(BlueprintNativeEvent, BlueprintPure, Category = "DamageCondition")
	FString GetDisplayString() const;

	virtual FString GetDisplayString_Implementation() const
	{
		FString Name = GetClass()->GetName();
		Name.RemoveFromEnd(TEXT("_C"));
		return Name;
	}
};
