// PipelineManager.h — UPipelineManager: 拓扑排序引擎 + Pipeline 执行器
#pragma once

#include "CoreMinimal.h"
#include "DPUFramework/DPUDefinition.h"
#include "DPUFramework/DamageContext.h"
#include "PipelineManager.generated.h"

class UPipelineAsset;

/**
 * 稳定拓扑排序的结果。
 */
USTRUCT(BlueprintType)
struct SAGASTATS_API FPipelineSortResult
{
	GENERATED_BODY()

	/** 按拓扑排序的执行顺序排列的 DPU */
	UPROPERTY()
	TArray<TObjectPtr<UDPUDefinition>> SortedDPUs;

	/** 如果检测到循环依赖则为 true（Pipeline 无法执行） */
	UPROPERTY()
	bool bHasCycle = false;

	/** 检测到的循环依赖的诊断信息 */
	UPROPERTY()
	TArray<FString> CycleInfo;
};

/**
 * 单个 DPU 的执行日志条目。
 */
USTRUCT(BlueprintType)
struct SAGASTATS_API FDPUExecutionEntry
{
	GENERATED_BODY()

	UPROPERTY()
	FName DPUName;

	UPROPERTY()
	bool bExecuted = false; // true = 已执行, false = 已跳过 (Condition 为 false)
};

/**
 * UPipelineManager — 运行时引擎。
 * 接收 DPU 子集 -> 稳定拓扑排序 -> 评估 Conditions -> 按序执行。
 * 编辑器预览和运行时执行使用相同代码。
 */
UCLASS(BlueprintType)
class SAGASTATS_API UPipelineManager : public UObject
{
	GENERATED_BODY()

public:
	// === 核心 API ===

	/**
	 * 稳定拓扑排序（Kahn 算法的 BFS 变体）。
	 * 静态方法，编辑器可以无需实例化管理器直接复用。
	 * 对于无依赖关系的 DPU，保留原始数组顺序。
	 */
	static FPipelineSortResult StableTopologicalSort(const TArray<UDPUDefinition*>& DPUs);

	/**
	 * 执行 Pipeline：排序 -> 评估条件 -> 执行逻辑。
	 * 返回执行日志。
	 */
	TArray<FDPUExecutionEntry> ExecutePipeline(const TArray<UDPUDefinition*>& ActiveDPUs, UDamageContext* DC);

	/** 便捷方法：从 PipelineAsset 执行。 */
	TArray<FDPUExecutionEntry> ExecuteFromAsset(UPipelineAsset* Asset, UDamageContext* DC);

private:
	/** 逻辑实例缓存，避免每次执行时重复创建 NewObject。 */
	UPROPERTY()
	TMap<UClass*, TObjectPtr<UDPULogicBase>> LogicInstances;

	UDPULogicBase* GetOrCreateLogic(TSubclassOf<UDPULogicBase> LogicClass);
};
