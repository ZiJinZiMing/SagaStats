// PipelineManager.cpp — 稳定拓扑排序 + Pipeline 执行 + Mermaid DAG 导出
#include "DPUFramework/PipelineManager.h"
#include "DPUFramework/PipelineAsset.h"
#include "DPUFramework/StringConditionExpression.h"
#include "SagaStatsLog.h"
#include "Misc/FileHelper.h"
#include "HAL/PlatformFileManager.h"
#include "Misc/Paths.h"

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

	// 4. 自动导出 Mermaid DAG
	if (bAutoExportMermaid)
	{
		ExportMermaidDAG(ActiveDPUs, ExecutionLog, DC);
	}

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

// ============================================================================
// Mermaid DAG 导出
// ============================================================================

void UPipelineManager::ExportMermaidDAG(
	const TArray<UDPUDefinition*>& DPUs,
	const TArray<FDPUExecutionEntry>& ExecutionLog,
	const UDamageContext* DC) const
{
	// 构建执行状态映射：DPUName -> bExecuted
	TMap<FName, bool> ExecStatusMap;
	for (const FDPUExecutionEntry& Entry : ExecutionLog)
	{
		ExecStatusMap.Add(Entry.DPUName, Entry.bExecuted);
	}

	// 构建生产者映射：字段名 -> 生产者 DPU 名称
	TMap<FName, FName> ProducerMap;
	for (const UDPUDefinition* DPU : DPUs)
	{
		if (!DPU) continue;
		for (const FName& Field : DPU->Produces)
		{
			ProducerMap.Add(Field, DPU->DPUName);
		}
	}

	// 收集 DC 初始字段（没有 DPU 生产者的字段）
	TArray<TPair<FName, FString>> InitialFields;
	if (DC)
	{
		for (const auto& Pair : DC->GetAllFields())
		{
			if (!ProducerMap.Contains(Pair.Key))
			{
				InitialFields.Add({Pair.Key, Pair.Value.ToString()});
			}
		}
	}

	// ---- 生成 Mermaid 内容 ----
	FString M;
	M += TEXT("graph LR\n");

	// 标题注释
	FString Label = ScenarioLabel.IsEmpty() ? TEXT("DPU Pipeline") : ScenarioLabel;
	M += FString::Printf(TEXT("    %%%% %s - %s\n\n"), *Label,
		*FDateTime::Now().ToString());

	// 样式定义
	M += TEXT("    classDef exec fill:#d4edda,stroke:#28a745,color:#000\n");
	M += TEXT("    classDef skip fill:#e2e3e5,stroke:#6c757d,color:#666\n");
	M += TEXT("    classDef initCtx fill:#fff3cd,stroke:#ffc107,color:#000\n");
	M += TEXT("\n");

	// DC 初始字段节点
	if (InitialFields.Num() > 0)
	{
		FString FieldLines;
		for (const auto& Pair : InitialFields)
		{
			if (!FieldLines.IsEmpty()) FieldLines += TEXT("<br/>");
			FieldLines += FString::Printf(TEXT("%s = %s"), *Pair.Key.ToString(), *Pair.Value);
		}
		M += FString::Printf(TEXT("    DC_Init[\"<b>DC Initial</b><br/>%s\"]:::initCtx\n"), *FieldLines);
		M += TEXT("\n");
	}

	// 拓扑排序获取执行顺序
	FPipelineSortResult SortResult = StableTopologicalSort(const_cast<TArray<UDPUDefinition*>&>(DPUs));

	// DPU 节点定义
	for (int32 i = 0; i < SortResult.SortedDPUs.Num(); i++)
	{
		const UDPUDefinition* DPU = SortResult.SortedDPUs[i];
		if (!DPU) continue;

		// 执行状态
		const bool* bExec = ExecStatusMap.Find(DPU->DPUName);
		bool bExecuted = bExec ? *bExec : false;
		FString StatusTag = bExecuted ? TEXT("EXEC") : TEXT("SKIP");
		FString StyleClass = bExecuted ? TEXT("exec") : TEXT("skip");

		// Condition 表达式文本
		FString CondText = TEXT("(none)");
		if (DPU->Condition)
		{
			if (const UStringConditionExpression* StrCond = Cast<UStringConditionExpression>(DPU->Condition))
			{
				CondText = StrCond->Expression;
			}
			else
			{
				CondText = TEXT("(C++ override)");
			}
		}
		// 转义 Mermaid 中的特殊字符
		CondText.ReplaceInline(TEXT("\""), TEXT("#quot;"));

		// Produces 字段列表
		FString ProducesText;
		for (const FName& Field : DPU->Produces)
		{
			if (!ProducesText.IsEmpty()) ProducesText += TEXT(", ");
			ProducesText += Field.ToString();
		}
		if (ProducesText.IsEmpty()) ProducesText = TEXT("(none)");

		// 节点定义：圆角矩形，多行标签
		M += FString::Printf(
			TEXT("    %s[\"%s [%s] #%d<br/>Cond: %s<br/>Produces: %s\"]:::%s\n"),
			*DPU->DPUName.ToString(),
			*DPU->DPUName.ToString(),
			*StatusTag,
			i,
			*CondText,
			*ProducesText,
			*StyleClass);
	}

	M += TEXT("\n");

	// 隐藏的执行顺序链：强制 Mermaid 按拓扑排序从左到右排列每个节点
	// DC_Init ~~~ #0 ~~~ #1 ~~~ #2 ~~~ ... （~~~ 是 Mermaid 的隐藏连线语法）
	M += TEXT("    %% 执行顺序链（隐藏连线，控制水平布局）\n");
	if (InitialFields.Num() > 0 && SortResult.SortedDPUs.Num() > 0)
	{
		M += FString::Printf(TEXT("    DC_Init ~~~ %s\n"),
			*SortResult.SortedDPUs[0]->DPUName.ToString());
	}
	for (int32 i = 0; i + 1 < SortResult.SortedDPUs.Num(); i++)
	{
		M += FString::Printf(TEXT("    %s ~~~ %s\n"),
			*SortResult.SortedDPUs[i]->DPUName.ToString(),
			*SortResult.SortedDPUs[i + 1]->DPUName.ToString());
	}
	M += TEXT("\n");

	// 依赖连线：Producer --字段名--> Consumer
	for (const UDPUDefinition* DPU : SortResult.SortedDPUs)
	{
		if (!DPU) continue;
		TArray<FName> ConsumedFields = DPU->GetConsumedFields();

		// 按来源 DPU 分组，合并同一条边上的多个字段名
		TMap<FName, TArray<FName>> EdgesFromProducer; // 生产者DPU名 -> 字段名列表
		TArray<FName> InitialFieldEdges; // 来自 DC 初始字段的

		for (const FName& Field : ConsumedFields)
		{
			FName* ProducerName = ProducerMap.Find(Field);
			if (ProducerName)
			{
				EdgesFromProducer.FindOrAdd(*ProducerName).Add(Field);
			}
			else
			{
				// 来自 DC 初始字段
				InitialFieldEdges.Add(Field);
			}
		}

		// DC 初始字段 -> DPU 的连线
		if (InitialFieldEdges.Num() > 0 && InitialFields.Num() > 0)
		{
			FString EdgeLabel;
			for (const FName& F : InitialFieldEdges)
			{
				if (!EdgeLabel.IsEmpty()) EdgeLabel += TEXT(", ");
				EdgeLabel += F.ToString();
			}
			M += FString::Printf(TEXT("    DC_Init -->|%s| %s\n"),
				*EdgeLabel, *DPU->DPUName.ToString());
		}

		// Producer DPU -> Consumer DPU 的连线
		for (const auto& Edge : EdgesFromProducer)
		{
			FString EdgeLabel;
			for (const FName& F : Edge.Value)
			{
				if (!EdgeLabel.IsEmpty()) EdgeLabel += TEXT(", ");
				EdgeLabel += F.ToString();
			}
			M += FString::Printf(TEXT("    %s -->|%s| %s\n"),
				*Edge.Key.ToString(), *EdgeLabel, *DPU->DPUName.ToString());
		}
	}

	// ---- 保存文件 ----
	FString SafeLabel = Label.Replace(TEXT(" "), TEXT("_")).Replace(TEXT("."), TEXT("_"));
	FString FileName = FString::Printf(TEXT("DPU_DAG_%s_%s.mmd"),
		*SafeLabel,
		*FDateTime::Now().ToString(TEXT("%Y%m%d_%H%M%S")));
	FString FilePath = FPaths::Combine(FPaths::ProjectSavedDir(), TEXT("Graphs"), FileName);

	// 确保目录存在
	FString DirPath = FPaths::GetPath(FilePath);
	if (!FPlatformFileManager::Get().GetPlatformFile().DirectoryExists(*DirPath))
	{
		FPlatformFileManager::Get().GetPlatformFile().CreateDirectoryTree(*DirPath);
	}

	if (FFileHelper::SaveStringToFile(M, *FilePath, FFileHelper::EEncodingOptions::ForceUTF8))
	{
		UE_LOG(LogSagaStats, Log, TEXT("Mermaid DAG 已保存: %s"), *FilePath);
	}
	else
	{
		UE_LOG(LogSagaStats, Error, TEXT("Mermaid DAG 保存失败: %s"), *FilePath);
	}
}
