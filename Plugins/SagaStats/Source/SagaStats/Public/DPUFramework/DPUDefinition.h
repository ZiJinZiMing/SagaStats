// DPUDefinition.h — UDPUDefinition: DPU 定义（DataAsset 形式）
#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "DPUFramework/DPULogicBase.h"
#include "DPUFramework/ConditionNode.h"
#include "DPUDefinition.generated.h"

/**
 * UDPUDefinition — DPU 的静态定义。
 *
 * v4.5: DPU 身份标识（DPUName）同时用于 DC 中 Fact 索引和 Condition 中的引用。
 * Produces 声明单个 Fact 类型（UScriptStruct*），不再是 Fact Key 列表。
 */
UCLASS(BlueprintType)
class SAGASTATS_API UDPUDefinition : public UDataAsset
{
	GENERATED_BODY()

public:
	/** DPU 身份标识——同时用于 DC 中 Fact 索引和 Condition 中的引用 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName DPUName;

	/** 人类可读的描述 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString Description;

	/** 此 DPU 产出的 Fact 类型（v4.5: 每个 DPU 产出一个 Fact） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<UScriptStruct> ProducesFactType;

	/** Condition 条件树（R4: 装配到 DPU 上，可为空 = 始终执行） */
	UPROPERTY(EditAnywhere, Instanced, BlueprintReadWrite)
	TObjectPtr<UConditionNode> Condition;

	/** 实现机制的逻辑类（UDPULogicBase 的子类） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<UDPULogicBase> LogicClass;

	/** 表现层选择的基础优先级 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float BasePriority = 0.f;

	/** 从 Condition 条件树中提取依赖的 DPU 名列表（用于拓扑排序） */
	TArray<FName> GetConsumedDPUs() const;
};
