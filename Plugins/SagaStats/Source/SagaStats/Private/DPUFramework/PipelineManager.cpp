// PipelineManager.cpp — 稳定拓扑排序 + Pipeline 执行
#include "DPUFramework/PipelineManager.h"
#include "DPUFramework/PipelineAsset.h"
#include "SagaStatsLog.h"

// ============================================================================
// 稳定拓扑排序（Kahn 算法的 BFS + 原始索引优先队列）
// ============================================================================

FPipelineSortResult UPipelineManager::StableTopologicalSort(const TArray<UDPUDefinition*>& DPUs)
{
	FPipelineSortResult Result;

	if (DPUs.Num() == 0)
	{
		return Result;
	}

	// 1. 分配原始索引
	TMap<UDPUDefinition*, int32> OriginalIndex;
	for (int32 i = 0; i < DPUs.Num(); i++)
	{
		OriginalIndex.Add(DPUs[i], i);
	}

	// 2. 构建生产者映射：字段名 -> 生产者 DPU
	TMap<FName, UDPUDefinition*> ProducerMap;
	for (UDPUDefinition* DPU : DPUs)
	{
		if (!DPU) continue;
		for (const FName& Field : DPU->Produces)
		{
			ProducerMap.Add(Field, DPU);
		}
	}

	// 3. 构建依赖图
	TMap<UDPUDefinition*, TSet<UDPUDefinition*>> Dependencies; // DPU -> 它依赖的 DPU 集合
	TMap<UDPUDefinition*, TSet<UDPUDefinition*>> Dependents;   // DPU -> 依赖它的 DPU 集合
	TMap<UDPUDefinition*, int32> InDegree;

	for (UDPUDefinition* DPU : DPUs)
	{
		if (!DPU) continue;
		Dependencies.FindOrAdd(DPU);
		Dependents.FindOrAdd(DPU);
		InDegree.FindOrAdd(DPU) = 0;
	}

	for (UDPUDefinition* DPU : DPUs)
	{
		if (!DPU) continue;
		TArray<FName> ConsumedFields = DPU->GetConsumedFields();
		for (const FName& Field : ConsumedFields)
		{
			UDPUDefinition** ProducerPtr = ProducerMap.Find(Field);
			if (ProducerPtr && *ProducerPtr != DPU) // 仅限活跃的生产者，排除自引用
			{
				UDPUDefinition* Producer = *ProducerPtr;
				if (!Dependencies[DPU].Contains(Producer))
				{
					Dependencies[DPU].Add(Producer);
					Dependents[Producer].Add(DPU);
					InDegree[DPU]++;
				}
			}
		}
	}

	// 4. 稳定 BFS：按原始索引排序的优先队列（最小优先）
	// 由于 DPU 数量较少 (<50)，使用简单的有序数组
	TArray<UDPUDefinition*> ReadyQueue;
	for (UDPUDefinition* DPU : DPUs)
	{
		if (!DPU) continue;
		if (InDegree[DPU] == 0)
		{
			ReadyQueue.Add(DPU);
		}
	}
	// 按原始索引排序（UE 的 Sort 会对指针数组自动解引用，lambda 收到引用）
	ReadyQueue.Sort([&OriginalIndex](const UDPUDefinition& A, const UDPUDefinition& B)
	{
		return OriginalIndex[const_cast<UDPUDefinition*>(&A)] < OriginalIndex[const_cast<UDPUDefinition*>(&B)];
	});

	TArray<UDPUDefinition*> Sorted;
	while (ReadyQueue.Num() > 0)
	{
		// 弹出原始索引最小的那个
		UDPUDefinition* Current = ReadyQueue[0];
		ReadyQueue.RemoveAt(0);
		Sorted.Add(Current);

		// 更新依赖者
		for (UDPUDefinition* Dependent : Dependents[Current])
		{
			InDegree[Dependent]--;
			if (InDegree[Dependent] == 0)
			{
				// 插入就绪队列并保持排序顺序
				int32 InsertIdx = 0;
				int32 DepIdx = OriginalIndex[Dependent];
				while (InsertIdx < ReadyQueue.Num() && OriginalIndex[ReadyQueue[InsertIdx]] < DepIdx)
				{
					InsertIdx++;
				}
				ReadyQueue.Insert(Dependent, InsertIdx);
			}
		}
	}

	// 5. 循环依赖检测
	if (Sorted.Num() < DPUs.Num())
	{
		Result.bHasCycle = true;
		for (UDPUDefinition* DPU : DPUs)
		{
			if (!DPU) continue;
			if (!Sorted.Contains(DPU))
			{
				FString DepNames;
				for (UDPUDefinition* Dep : Dependencies[DPU])
				{
					if (!Sorted.Contains(Dep))
					{
						if (!DepNames.IsEmpty()) DepNames += TEXT(", ");
						DepNames += Dep->DPUName.ToString();
					}
				}
				Result.CycleInfo.Add(FString::Printf(TEXT("%s depends on [%s]"),
					*DPU->DPUName.ToString(), *DepNames));
			}
		}
	}

	for (UDPUDefinition* DPU : Sorted)
	{
		Result.SortedDPUs.Add(DPU);
	}

	return Result;
}

// ============================================================================
// Pipeline 执行
// ============================================================================

TArray<FDPUExecutionEntry> UPipelineManager::ExecutePipeline(
	const TArray<UDPUDefinition*>& ActiveDPUs, UDamageContext* DC)
{
	TArray<FDPUExecutionEntry> ExecutionLog;

	// 1. 拓扑排序
	FPipelineSortResult SortResult = StableTopologicalSort(ActiveDPUs);

	if (SortResult.bHasCycle)
	{
		UE_LOG(LogSagaStats, Error, TEXT("Pipeline has circular dependencies:"));
		for (const FString& Info : SortResult.CycleInfo)
		{
			UE_LOG(LogSagaStats, Error, TEXT("  %s"), *Info);
		}
		return ExecutionLog;
	}

	// 记录排序结果
	{
		FString SortOrder;
		for (const auto& DPU : SortResult.SortedDPUs)
		{
			if (!SortOrder.IsEmpty()) SortOrder += TEXT(" -> ");
			SortOrder += DPU->DPUName.ToString();
		}
		UE_LOG(LogSagaStats, Log, TEXT("Pipeline sort order: %s"), *SortOrder);
	}

	// 2. 按顺序执行每个 DPU
	for (UDPUDefinition* DPU : SortResult.SortedDPUs)
	{
		if (!DPU) continue;

		FDPUExecutionEntry Entry;
		Entry.DPUName = DPU->DPUName;

		// 评估 Condition
		if (DPU->Condition)
		{
			if (!DPU->Condition->Evaluate(DC))
			{
				Entry.bExecuted = false;
				ExecutionLog.Add(Entry);
				UE_LOG(LogSagaStats, Log, TEXT("  [SKIP] %s (Condition false)"), *DPU->DPUName.ToString());
				continue;
			}
		}

		// 执行逻辑
		if (DPU->LogicClass)
		{
			UDPULogicBase* Logic = GetOrCreateLogic(DPU->LogicClass);
			if (Logic)
			{
				Logic->Execute(DC);
			}
		}

		Entry.bExecuted = true;
		ExecutionLog.Add(Entry);
		UE_LOG(LogSagaStats, Log, TEXT("  [EXEC] %s"), *DPU->DPUName.ToString());
	}

	// 3. 记录最终 DC 状态
	UE_LOG(LogSagaStats, Log, TEXT("%s"), *DC->DumpToString());

	return ExecutionLog;
}

TArray<FDPUExecutionEntry> UPipelineManager::ExecuteFromAsset(
	UPipelineAsset* Asset, UDamageContext* DC)
{
	if (!Asset)
	{
		UE_LOG(LogSagaStats, Error, TEXT("ExecuteFromAsset: null PipelineAsset"));
		return {};
	}

	TArray<UDPUDefinition*> RawPtrs;
	for (const auto& Def : Asset->DPUDefinitions)
	{
		RawPtrs.Add(Def.Get());
	}

	return ExecutePipeline(RawPtrs, DC);
}

UDPULogicBase* UPipelineManager::GetOrCreateLogic(TSubclassOf<UDPULogicBase> LogicClass)
{
	if (!LogicClass)
	{
		return nullptr;
	}

	UClass* ClassPtr = LogicClass.Get();
	if (TObjectPtr<UDPULogicBase>* Found = LogicInstances.Find(ClassPtr))
	{
		return Found->Get();
	}

	UDPULogicBase* NewLogic = NewObject<UDPULogicBase>(this, ClassPtr);
	LogicInstances.Add(ClassPtr, NewLogic);
	return NewLogic;
}
