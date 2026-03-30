// PipelineAsset.h — UPipelineAsset: 自洽的 Pipeline 定义 + 执行引擎
#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "DPUFramework/DPUDefinition.h"
#include "DPUFramework/DamageContext.h"
#include "PipelineAsset.generated.h"

/**
 * 稳定拓扑排序的结果。
 */
USTRUCT(BlueprintType)
struct SAGASTATS_API FPipelineSortResult
{
	GENERATED_BODY()

	/** 按拓扑排序的执行顺序排列的 DPU */
	UPROPERTY(BlueprintReadOnly)
	TArray<TObjectPtr<UDPUDefinition>> SortedDPUs;

	/** 如果检测到循环依赖则为 true */
	UPROPERTY(BlueprintReadOnly)
	bool bHasCycle = false;

	/** 循环依赖的诊断信息 */
	UPROPERTY(BlueprintReadOnly)
	TArray<FString> CycleInfo;
};

/**
 * 单个 DPU 的执行日志条目。
 */
USTRUCT(BlueprintType)
struct SAGASTATS_API FDPUExecutionEntry
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	FName DPUName;

	UPROPERTY(BlueprintReadOnly)
	bool bExecuted = false;
};

/**
 * UPipelineAsset — 自洽的 Pipeline 定义 + 执行引擎。
 *
 * 存储一组 DPU 定义，自带 Build()（拓扑排序烘焙）和 Execute()（管线执行）。
 * 不需要外部的 PipelineManager。
 *
 * 两种来源：编辑器构建（固定 Pipeline）或运行时组装（动态 Pipeline）。
 * 两者产生相同的资产类型——运行时引擎不做区分。
 */
UCLASS(BlueprintType)
class SAGASTATS_API UPipelineAsset : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName PipelineName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<TObjectPtr<UDPUDefinition>> DPUDefinitions;

	// =====================================================================
	// 核心 API
	// =====================================================================

	/**
	 * 拓扑排序烘焙。根据 DPUDefinitions 的产销关系构建 DAG，缓存结果。
	 * 编辑器下可序列化保存；运行时缓存在内存。
	 * DPU 集合未变时，重复 Execute() 复用缓存。
	 */
	UFUNCTION(BlueprintCallable, Category = "DPU Pipeline")
	FPipelineSortResult Build();

	/**
	 * 执行管线。未烘焙则自动 Build()。
	 * 按缓存的拓扑顺序：评估 Condition → 执行 DPU Logic → 返回执行日志。
	 */
	UFUNCTION(BlueprintCallable, Category = "DPU Pipeline")
	TArray<FDPUExecutionEntry> Execute(UDamageContext* DC);

	/**
	 * 稳定拓扑排序（Kahn 算法 BFS 变体）。
	 * 静态方法，编辑器可复用。无依赖 DPU 保留原始数组顺序。
	 */
	UFUNCTION(BlueprintCallable, Category = "DPU Pipeline")
	static FPipelineSortResult StableTopologicalSort(const TArray<UDPUDefinition*>& DPUs);

	/** 是否已烘焙 */
	UPROPERTY(BlueprintReadOnly)
	bool bIsBaked = false;

#if WITH_EDITOR
	/** 编辑器中修改 DPUDefinitions 或其内容时，自动置 bIsBaked = false */
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
	virtual void PostEditChangeChainProperty(FPropertyChangedChainEvent& PropertyChangedEvent) override;
#endif

	// =====================================================================
	// Mermaid DAG 导出
	// =====================================================================

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bAutoExportMermaid = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString ScenarioLabel;

	void ExportMermaidDAG(const TArray<FDPUExecutionEntry>& ExecutionLog,
		const UDamageContext* DC) const;

private:
	/** 烘焙后的排序结果 */
	UPROPERTY()
	TArray<TObjectPtr<UDPUDefinition>> SortedDPUs;

	/** DPU 逻辑实例缓存 */
	UPROPERTY()
	TMap<UClass*, TObjectPtr<UDPULogicBase>> LogicInstances;

	UDPULogicBase* GetOrCreateLogic(TSubclassOf<UDPULogicBase> LogicClass);
};
