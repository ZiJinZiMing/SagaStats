// DPULogicBase.h — DPU 机制逻辑基类
#pragma once

#include "CoreMinimal.h"
#include "StructUtils/InstancedStruct.h"
#include "DPULogicBase.generated.h"

class UDamageContext;

/**
 * UDPULogicBase — DPU 逻辑的抽象基类。
 *
 * v4.5: Execute 是纯函数——读 DC（只读）、执行逻辑、返回 Fact（FInstancedStruct）。
 * DPU 不负责写入 DC。PipelineAsset 拿到返回值后以 DPU 身份标识写入 DC。
 *
 * R2: DPU 不知道自己的身份标识，不知道 Condition，不知道其他 DPU。
 *
 * C++ 子类：重写 Execute_Implementation()。
 * Blueprint 子类：重写 Execute 事件，通过 Return 节点返回 FInstancedStruct。
 */
UCLASS(Abstract, Blueprintable)
class SAGASTATS_API UDPULogicBase : public UObject
{
	GENERATED_BODY()

public:
	/**
	 * 读取 DC（只读）→ 执行机制逻辑 → 返回 Fact（FInstancedStruct）。
	 * 不写入 DC——PipelineAsset 负责以 DPU 身份标识写入。
	 */
	UFUNCTION(BlueprintNativeEvent, Category = "DPU")
	FInstancedStruct Execute(const UDamageContext* DC);

	virtual FInstancedStruct Execute_Implementation(const UDamageContext* DC) { return FInstancedStruct(); }
};
