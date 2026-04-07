// DPULogicBase.h — DPU 机制逻辑基类
#pragma once

#include "CoreMinimal.h"
#include "StructUtils/InstancedStruct.h"
#include "DPULogicBase.generated.h"

class UDamageContext;

/**
 * UDPULogicBase — DPU 逻辑的抽象基类。
 *
 * v4.8: Execute(DC, OutFact) — 框架传入默认初始化的 OutFact，Logic 只需填字段。
 * PipelineAsset 负责创建 OutFact 和写入 DC。Logic 不感知 DC 写入。
 */
UCLASS(Abstract, Blueprintable)
class SAGASTATS_API UDPULogicBase : public UObject
{
	GENERATED_BODY()

public:
	/** 此 Logic 产出的 Fact 类型（蓝图子类在类默认值中设置；C++ 子类可 override） */
	UPROPERTY(EditAnywhere, Category = "DPU", meta = (DisplayName = "Produces Fact Type"))
	UScriptStruct* ProducesFactType = nullptr;

	virtual UScriptStruct* GetProducesFactType() const { return ProducesFactType; }

	/**
	 * 执行机制逻辑。
	 * @param DC       共享上下文（读取事件上下文和上游 Fact）
	 * @param OutFact  框架预创建的输出 Fact（已按 ProducesFactType 默认初始化），Logic 填字段即可
	 */
	UFUNCTION(BlueprintNativeEvent, Category = "DPU")
	void Execute(UDamageContext* DC, UPARAM(ref) FInstancedStruct& OutFact);

	virtual void Execute_Implementation(UDamageContext* DC, UPARAM(ref) FInstancedStruct& OutFact) {}
};
