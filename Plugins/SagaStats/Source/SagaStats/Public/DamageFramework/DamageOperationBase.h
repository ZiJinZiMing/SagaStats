// DamageOperationBase.h — DamageOperation 机制逻辑基类
#pragma once

#include "CoreMinimal.h"
#include "StructUtils/InstancedStruct.h"
#include "DamageOperationBase.generated.h"

class UDamageContext;

/**
 * UDamageOperationBase — DamageOperation 逻辑的抽象基类。
 *
 * v4.8: Execute(Context, OutEffect) — 框架传入默认初始化的 OutEffect，Operation 只需填字段。
 * DamagePipeline 负责创建 OutEffect 和写入 Context。Operation 不感知 Context 写入。
 */
UCLASS(Abstract, Blueprintable)
class SAGASTATS_API UDamageOperationBase : public UObject
{
	GENERATED_BODY()

public:
	/** 此 Operation 产出的 Effect 类型（蓝图子类在类默认值中设置；C++ 子类可 override） */
	UPROPERTY(EditAnywhere, Category = "DamageRule", meta = (DisplayName = "Produces Effect Type"))
	UScriptStruct* ProducesEffectType = nullptr;

	virtual UScriptStruct* GetProducesEffectType() const { return ProducesEffectType; }

	/**
	 * 执行机制逻辑。
	 * @param Context    共享上下文（读取事件上下文和上游 Effect）
	 * @param OutEffect  框架预创建的输出 Effect（已按 ProducesEffectType 默认初始化），Operation 填字段即可
	 */
	UFUNCTION(BlueprintNativeEvent, Category = "DamageRule")
	void Execute(UDamageContext* Context, UPARAM(ref) FInstancedStruct& OutEffect);

	virtual void Execute_Implementation(UDamageContext* Context, UPARAM(ref) FInstancedStruct& OutEffect) {}
};
