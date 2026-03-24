// DPULogicBase.h — DPU 机制逻辑基类
#pragma once

#include "CoreMinimal.h"
#include "DPULogicBase.generated.h"

class UDamageContext;

/**
 * UDPULogicBase — DPU 逻辑的抽象基类。
 * 继承并重写 Execute() 以实现具体的机制逻辑。
 * R2: DPU 是自包含的。不知道 Condition 或其他 DPU 的存在。
 * R1: 仅通过 DC 进行通信。
 */
UCLASS(Abstract, Blueprintable)
class SAGASTATS_API UDPULogicBase : public UObject
{
	GENERATED_BODY()

public:
	/** 读取 DC -> 执行机制逻辑 -> 将结果写回 DC。 */
	virtual void Execute(UDamageContext* DC) PURE_VIRTUAL(UDPULogicBase::Execute, );
};
