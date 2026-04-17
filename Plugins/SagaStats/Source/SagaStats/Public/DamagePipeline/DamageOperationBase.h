/***************************************************************************************************************
* Plugin:       SagaStats
* Author:       Jinming Zhang
* Description:  SagaStats offers modular damage process and meter systems to support adaptable status management
****************************************************************************************************************/

// DamageOperationBase.h — DamageOperation 机制逻辑基类
#pragma once

#include "CoreMinimal.h"
#include "StructUtils/InstancedStruct.h"
#include "DamagePipeline/DamageContext.h"
#include "DamageOperationBase.generated.h"

/**
 * UDamageOperationBase — DamageOperation 逻辑的抽象基类。
 *
 * Execute(Context, OutEffect) — 框架传入默认初始化的 OutEffect，Operation 只需填字段。
 * DamagePipeline 负责创建 OutEffect 和写入 Context。Operation 不感知 Context 写入。
 *
 * ============================================================================
 * EffectType 声明：C++ / 蓝图 双路径（对齐 UDamageCondition_Effect 的模式）
 * ============================================================================
 *
 * Operation 子类必须声明自己产出的 Effect 类型。C++ 和蓝图**路径对齐**——两者都通过
 * "设置 EffectType 字段"表达；基类默认 GetEffectType() 读取字段。
 *
 * - **C++ 子类**（推荐，构造函数赋字段）：
 *     UCLASS()
 *     class UDamageOperation_Guard : public UDamageOperationBase
 *     {
 *         UDamageOperation_Guard() { EffectType = FGuardEffect::StaticStruct(); }
 *         virtual void Execute_Implementation(UDamageContext* Ctx, FInstancedStruct& OutEffect) override;
 *     };
 *
 * - **蓝图子类**：
 *     右键菜单 "Damage Operation (Blueprint)" → 在 Class Defaults 里设置 `EffectType` 字段
 *     → override Execute 事件。
 *
 * EffectType 字段在非 CDO 实例上**自动隐藏**（EditCondition="IsClassDefaultContext" +
 * EditConditionHides）——防止在 DamageRule 装配时误改（EffectType 是类级属性，不是实例级）。
 * 高级用法：override GetEffectType() 返回动态类型（罕见）。
 */
UCLASS(Abstract, Blueprintable)
class SAGASTATS_API UDamageOperationBase : public UObject
{
	GENERATED_BODY()

public:

	/**
	 * 本 Operation 产出的 Effect 类型。
	 * C++ 子类：override 返回 `FXxxEffect::StaticStruct()`（推荐）。
	 * 蓝图子类：不 override，在 Class Defaults 里设置 EffectType 字段，基类默认实现读取它。
	 */
	virtual UScriptStruct* GetEffectType() const { return EffectType; }

	/**
	 * 执行机制逻辑。
	 * @param Context    共享上下文（读取事件上下文和上游 Effect）
	 * @param OutEffect  处理输出的 Effect ,类型根据 EffectType 确定
	 */
	UFUNCTION(BlueprintNativeEvent, Category = "DamageRule")
	void Execute(UDamageContext* Context, UPARAM(ref) FInstancedStruct& OutEffect);

	virtual void Execute_Implementation(UDamageContext* Context, UPARAM(ref) FInstancedStruct& OutEffect) {}

	/**
	 * 子类读取上游 Effect 的便利接口。基类是 UDamageContext 的 friend，能访问 protected GetEffect。
	 *
	 * 注意：本接口**不做 R5 产销依赖校验**——Operation 目前可读任意 Effect 而不声明依赖（已知漏洞）。
	 * R5 强制留到后续：Operation 侧需要引入 ConsumesEffectTypes 声明 + Build 时校验。
	 */
	template<typename T>
	static const T* ReadEffect(const UDamageContext* Context)
	{
		return Context ? Context->GetEffect<T>() : nullptr;
	}

protected:
	/**
	 * 蓝图子类在 Blueprint Class Defaults 中设置；C++ 子类通过 override `GetEffectType()` 替代。
	 *
	 * 非 CDO 实例上通过 IsClassDefaultContext() + EditConditionHides 完全隐藏，
	 * 防止在 DamageRule 装配 OperationClass 时把本字段当作实例级属性误改。
	 */
	UPROPERTY(EditAnywhere, Category = "DamageRule",
		meta = (EditCondition = "IsClassDefaultContext", EditConditionHides))
	UScriptStruct* EffectType = nullptr;

private:
	/** EditCondition 驱动函数：CDO/Archetype 返回 true → EffectType 可见可编辑；普通实例返回 false → 隐藏 */
	UFUNCTION()
	bool IsClassDefaultContext() const { return HasAnyFlags(RF_ClassDefaultObject | RF_ArchetypeObject); }
};
