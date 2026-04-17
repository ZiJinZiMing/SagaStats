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
 */
UCLASS(Abstract, Blueprintable)
class SAGASTATS_API UDamageOperationBase : public UObject
{
	GENERATED_BODY()

public:

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
	/** 此 Operation 产出的 Effect 类型（蓝图子类在类默认值中设置；C++ 子类可 override） */
	UPROPERTY(EditDefaultsOnly, Category = "DamageRule")
	UScriptStruct* EffectType = nullptr;
};
