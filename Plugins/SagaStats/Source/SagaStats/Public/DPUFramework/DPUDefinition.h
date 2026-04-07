// DPUDefinition.h — UDPUDefinition: DPU 定义（DataAsset 形式）
#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "DPUFramework/DPULogicBase.h"
#include "DPUFramework/DPUPredicate.h"
#include "DPUDefinition.generated.h"

/**
 * UDPUDefinition — DPU 的静态定义。
 *
 * v4.7: ProducesFactType 从 LogicClass CDO 获取，不再在 Definition 上声明。
 * Condition 类型改为 UDPUPredicate（谓词容器）。
 */
UCLASS(BlueprintType)
class SAGASTATS_API UDPUDefinition : public UDataAsset
{
	GENERATED_BODY()

public:
	/** DPU 身份标识——同时用于 DC 中 Fact 索引和依赖引用 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName DPUName;

	/** 人类可读的描述 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString Description;

	/** Condition 谓词容器（v4.7: Predicate/Condition 双层，可为空 = 始终执行） */
	UPROPERTY(EditAnywhere, Instanced, BlueprintReadWrite)
	TObjectPtr<UDPUPredicate> Condition;

	/** 实现机制的逻辑类（UDPULogicBase 的子类） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<UDPULogicBase> LogicClass;

	/** 表现层选择的基础优先级 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float BasePriority = 0.f;

	/** 从 LogicClass CDO 获取此 DPU 产出的 Fact 类型 */
	UScriptStruct* GetProducesFactType() const;

	/** 从 Condition 谓词树中提取依赖的 FactType 列表（用于拓扑排序） */
	TArray<UScriptStruct*> GetConsumedFactTypes() const;
};
