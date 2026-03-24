// DPUDefinition.h — UDPUDefinition: DPU 定义（DataAsset 形式）
#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "DPUFramework/DPULogicBase.h"
#include "DPUFramework/ConditionExpression.h"
#include "DPUDefinition.generated.h"

/**
 * UDPUDefinition — DPU 的静态定义。
 * 存储 Produces 声明、Condition（装配式，R4）、逻辑类引用、优先级。
 * 可在编辑器中创建为 DataAsset，也可在运行时构造。
 */
UCLASS(BlueprintType)
class SAGASTATS_API UDPUDefinition : public UDataAsset
{
	GENERATED_BODY()

public:
	/** 标识此 DPU 的唯一名称 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName DPUName;

	/** 人类可读的描述 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString Description;

	/** 此 DPU 写入的 DC 字段（Produces 声明，R5） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FName> Produces;

	/** Condition 门控 (R4: 装配到 DPU 上，可为空 = 始终执行) */
	UPROPERTY(EditAnywhere, Instanced, BlueprintReadWrite)
	TObjectPtr<UConditionExpression> Condition;

	/** 实现机制的逻辑类（UDPULogicBase 的子类） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<UDPULogicBase> LogicClass;

	/** 表现层选择的基础优先级（Phase 1.5 将使用此值） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 BasePriority = 0;

	/** 从 Condition 中提取消费的 DC 字段名（用于拓扑排序） */
	TArray<FName> GetConsumedFields() const;
};
