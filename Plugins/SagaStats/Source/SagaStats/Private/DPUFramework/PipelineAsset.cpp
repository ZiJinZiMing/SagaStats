// PipelineAsset.cpp — 自洽的 Pipeline：拓扑排序烘焙 + 执行 + Mermaid DAG 导出
#include "DPUFramework/PipelineAsset.h"
#include "DPUFramework/ConditionNode.h"
#include "SagaStatsLog.h"
#include "Misc/FileHelper.h"
#include "HAL/PlatformFileManager.h"
#include "Misc/Paths.h"

// ============================================================================
// 编辑器：属性变化时置 bIsBaked = false
// ============================================================================

#if WITH_EDITOR
void UPipelineAsset::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	if (bIsBaked)
	{
		bIsBaked = false;
		UE_LOG(LogSagaStats, Log, TEXT("PipelineAsset: 属性已修改，bIsBaked 置为 false，需要重新 Build"));
	}
}

void UPipelineAsset::PostEditChangeChainProperty(FPropertyChangedChainEvent& PropertyChangedEvent)
{
	Super::PostEditChangeChainProperty(PropertyChangedEvent);

	if (bIsBaked)
	{
		bIsBaked = false;
		UE_LOG(LogSagaStats, Log, TEXT("PipelineAsset: 嵌套属性已修改，bIsBaked 置为 false，需要重新 Build"));
	}
}
#endif

// ============================================================================
// 稳定拓扑排序（Kahn 算法 BFS + 原始索引优先队列）
// ============================================================================

FPipelineSortResult UPipelineAsset::StableTopologicalSort(const TArray<UDPUDefinition*>& DPUs)
{
	FPipelineSortResult Result;
	if (DPUs.Num() == 0) return Result;

	// 1. 分配原始索引
	TMap<UDPUDefinition*, int32> OriginalIndex;
	for (int32 i = 0; i < DPUs.Num(); i++)
	{
		OriginalIndex.Add(DPUs[i], i);
	}

	// 2. 构建生产者映射：Fact Key -> 生产者 DPU
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
	TMap<UDPUDefinition*, TSet<UDPUDefinition*>> Dependencies;
	TMap<UDPUDefinition*, TSet<UDPUDefinition*>> Dependents;
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
		TArray<FName> ConsumedFacts = DPU->GetConsumedFacts();
		for (const FName& Fact : ConsumedFacts)
		{
			UDPUDefinition** ProducerPtr = ProducerMap.Find(Fact);
			if (ProducerPtr && *ProducerPtr != DPU)
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

	// 4. 稳定 BFS
	TArray<UDPUDefinition*> ReadyQueue;
	for (UDPUDefinition* DPU : DPUs)
	{
		if (!DPU) continue;
		if (InDegree[DPU] == 0)
		{
			ReadyQueue.Add(DPU);
		}
	}
	ReadyQueue.Sort([&OriginalIndex](const UDPUDefinition& A, const UDPUDefinition& B)
	{
		return OriginalIndex[const_cast<UDPUDefinition*>(&A)] < OriginalIndex[const_cast<UDPUDefinition*>(&B)];
	});

	TArray<UDPUDefinition*> Sorted;
	while (ReadyQueue.Num() > 0)
	{
		UDPUDefinition* Current = ReadyQueue[0];
		ReadyQueue.RemoveAt(0);
		Sorted.Add(Current);

		for (UDPUDefinition* Dependent : Dependents[Current])
		{
			InDegree[Dependent]--;
			if (InDegree[Dependent] == 0)
			{
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

	// 5. 环检测
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
// Build：拓扑排序烘焙
// ============================================================================

FPipelineSortResult UPipelineAsset::Build()
{
	TArray<UDPUDefinition*> RawPtrs;
	for (const auto& Def : DPUDefinitions)
	{
		if (Def) RawPtrs.Add(Def.Get());
	}

	FPipelineSortResult Result = StableTopologicalSort(RawPtrs);

	SortedDPUs.Empty();
	for (const auto& DPU : Result.SortedDPUs)
	{
		SortedDPUs.Add(DPU.Get());
	}

	bIsBaked = !Result.bHasCycle;

	if (Result.bHasCycle)
	{
		UE_LOG(LogSagaStats, Error, TEXT("Pipeline Build 检测到循环依赖:"));
		for (const FString& Info : Result.CycleInfo)
		{
			UE_LOG(LogSagaStats, Error, TEXT("  %s"), *Info);
		}
	}
	else
	{
		FString SortOrder;
		for (const auto& DPU : SortedDPUs)
		{
			if (!SortOrder.IsEmpty()) SortOrder += TEXT(" -> ");
			SortOrder += DPU->DPUName.ToString();
		}
		UE_LOG(LogSagaStats, Log, TEXT("Pipeline Build 完成: %s"), *SortOrder);
	}

	return Result;
}

// ============================================================================
// Execute：管线执行
// ============================================================================

TArray<FDPUExecutionEntry> UPipelineAsset::Execute(UDamageContext* DC)
{
	TArray<FDPUExecutionEntry> ExecutionLog;

	if (!bIsBaked)
	{
		Build();
	}

	if (!bIsBaked)
	{
		UE_LOG(LogSagaStats, Error, TEXT("Pipeline 未烘焙（可能有循环依赖），无法执行"));
		return ExecutionLog;
	}

	for (UDPUDefinition* DPU : SortedDPUs)
	{
		if (!DPU) continue;

		FDPUExecutionEntry Entry;
		Entry.DPUName = DPU->DPUName;

		// 评估 Condition（调用 EvaluateCondition 以应用 bReverse）
		if (DPU->Condition)
		{
			if (!DPU->Condition->EvaluateCondition(DC))
			{
				Entry.bExecuted = false;
				ExecutionLog.Add(Entry);
				UE_LOG(LogSagaStats, Log, TEXT("  [SKIP] %s"), *DPU->DPUName.ToString());
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

	UE_LOG(LogSagaStats, Log, TEXT("%s"), *DC->DumpToString());

	if (bAutoExportMermaid)
	{
		ExportMermaidDAG(ExecutionLog, DC);
	}

	return ExecutionLog;
}

UDPULogicBase* UPipelineAsset::GetOrCreateLogic(TSubclassOf<UDPULogicBase> LogicClass)
{
	if (!LogicClass) return nullptr;

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

static const TCHAR* FieldColorPalette[] = {
	TEXT("#e6194b"), TEXT("#3cb44b"), TEXT("#4363d8"), TEXT("#f58231"),
	TEXT("#911eb4"), TEXT("#42d4f4"), TEXT("#f032e6"), TEXT("#bfef45"),
	TEXT("#fabed4"), TEXT("#469990"), TEXT("#dcbeff"), TEXT("#9a6324"),
	TEXT("#fffac8"), TEXT("#800000"), TEXT("#aaffc3"), TEXT("#808000"),
	TEXT("#ffd8b1"), TEXT("#000075"), TEXT("#a9a9a9"), TEXT("#e6beff"),
};
static const int32 FieldColorPaletteSize = UE_ARRAY_COUNT(FieldColorPalette);

void UPipelineAsset::ExportMermaidDAG(
	const TArray<FDPUExecutionEntry>& ExecutionLog,
	const UDamageContext* DC) const
{
	// 执行状态映射
	TMap<FName, bool> ExecStatusMap;
	for (const FDPUExecutionEntry& Entry : ExecutionLog)
	{
		ExecStatusMap.Add(Entry.DPUName, Entry.bExecuted);
	}

	// 生产者映射
	TMap<FName, FName> ProducerMap;
	for (const auto& DPU : SortedDPUs)
	{
		if (!DPU) continue;
		for (const FName& Field : DPU->Produces)
		{
			ProducerMap.Add(Field, DPU->DPUName);
		}
	}

	// 字段颜色分配
	TMap<FName, FString> FieldColorMap;
	{
		TArray<FName> OrderedFields;
		for (const auto& DPU : SortedDPUs)
		{
			if (!DPU) continue;
			for (const FName& F : DPU->Produces)
			{
				if (!FieldColorMap.Contains(F))
				{
					FieldColorMap.Add(F, FieldColorPalette[OrderedFields.Num() % FieldColorPaletteSize]);
					OrderedFields.Add(F);
				}
			}
			TArray<FName> Consumed = DPU->GetConsumedFacts();
			for (const FName& F : Consumed)
			{
				if (!FieldColorMap.Contains(F))
				{
					FieldColorMap.Add(F, FieldColorPalette[OrderedFields.Num() % FieldColorPaletteSize]);
					OrderedFields.Add(F);
				}
			}
		}
	}

	// DC 初始字段
	TArray<TPair<FName, FString>> InitialFields;
	if (DC)
	{
		for (const auto& Pair : DC->GetAllFacts())
		{
			if (!ProducerMap.Contains(Pair.Key))
			{
				// 从 FInstancedStruct 提取显示字符串
				FString ValueStr;
				if (!Pair.Value.IsValid())
				{
					ValueStr = TEXT("(invalid)");
				}
				else
				{
					// 简单显示：使用 DC 的标量接口
					if (DC->Contains(Pair.Key))
					{
						// 尝试各种标量类型
						bool bVal = DC->GetBool(Pair.Key);
						float fVal = DC->GetFloat(Pair.Key);
						if (fVal != 0.f)
							ValueStr = FString::SanitizeFloat(fVal);
						else
							ValueStr = bVal ? TEXT("true") : TEXT("false");
					}
				}
				InitialFields.Add({Pair.Key, ValueStr});
			}
		}
	}

	// ---- 生成 Mermaid ----
	FString M;
	M += TEXT("graph LR\n");

	FString Label = ScenarioLabel.IsEmpty() ? TEXT("DPU Pipeline") : ScenarioLabel;
	M += FString::Printf(TEXT("    %%%% %s - %s\n\n"), *Label, *FDateTime::Now().ToString());

	M += TEXT("    classDef exec fill:#d4edda,stroke:#28a745,color:#000\n");
	M += TEXT("    classDef skip fill:#e2e3e5,stroke:#6c757d,color:#666\n");
	M += TEXT("    classDef initCtx fill:#fff3cd,stroke:#ffc107,color:#000\n");
	M += TEXT("    classDef finalCtx fill:#cce5ff,stroke:#004085,color:#000\n\n");

	// DC Init 节点
	if (InitialFields.Num() > 0)
	{
		FString FieldLines;
		for (const auto& Pair : InitialFields)
		{
			if (!FieldLines.IsEmpty()) FieldLines += TEXT("<br/>");
			FieldLines += FString::Printf(TEXT("%s = %s"), *Pair.Key.ToString(), *Pair.Value);
		}
		M += FString::Printf(TEXT("    DC_Init[\"<b>DC Initial</b><br/>%s\"]:::initCtx\n\n"), *FieldLines);
	}

	// DPU 节点
	for (int32 i = 0; i < SortedDPUs.Num(); i++)
	{
		const UDPUDefinition* DPU = SortedDPUs[i];
		if (!DPU) continue;

		const bool* bExec = ExecStatusMap.Find(DPU->DPUName);
		bool bExecuted = bExec ? *bExec : false;
		FString StyleClass = bExecuted ? TEXT("exec") : TEXT("skip");

		// Condition 文本：使用条件树的 GetDisplayString()
		FString CondText = TEXT("(none)");
		if (DPU->Condition)
		{
			CondText = DPU->Condition->GetDisplayString();
		}
		CondText.ReplaceInline(TEXT("&&"), TEXT(" #amp;#amp; "));
		CondText.ReplaceInline(TEXT("||"), TEXT(" #124;#124; "));
		CondText.ReplaceInline(TEXT("\""), TEXT("#quot;"));

		// Produces 色块
		FString ProducesText;
		if (DPU->Produces.Num() == 0)
		{
			ProducesText = TEXT("(none)");
		}
		else
		{
			for (const FName& Field : DPU->Produces)
			{
				if (!ProducesText.IsEmpty()) ProducesText += TEXT("<br/>");
				FString FieldColor = FieldColorMap.FindRef(Field);
				if (!FieldColor.IsEmpty())
				{
					ProducesText += FString::Printf(TEXT("<font color='%s'>#9632;</font> %s"),
						*FieldColor, *Field.ToString());
				}
				else
				{
					ProducesText += Field.ToString();
				}
			}
		}

		M += FString::Printf(
			TEXT("    %s[\"%s #%d<br/>Cond: %s<br/>Produces:<br/>%s\"]:::%s\n"),
			*DPU->DPUName.ToString(), *DPU->DPUName.ToString(), i,
			*CondText, *ProducesText, *StyleClass);
	}

	M += TEXT("\n");

	// 连线
	int32 LinkIndex = 0;
	struct FColoredLink { int32 Index; FString Color; };
	TArray<FColoredLink> ColoredLinks;

	// 隐藏执行顺序链
	if (InitialFields.Num() > 0 && SortedDPUs.Num() > 0)
	{
		M += FString::Printf(TEXT("    DC_Init ~~~ %s\n"), *SortedDPUs[0]->DPUName.ToString());
		LinkIndex++;
	}
	for (int32 i = 0; i + 1 < SortedDPUs.Num(); i++)
	{
		M += FString::Printf(TEXT("    %s ~~~ %s\n"),
			*SortedDPUs[i]->DPUName.ToString(), *SortedDPUs[i + 1]->DPUName.ToString());
		LinkIndex++;
	}
	if (SortedDPUs.Num() > 0)
	{
		M += FString::Printf(TEXT("    %s ~~~ DC_Final\n"), *SortedDPUs.Last()->DPUName.ToString());
		LinkIndex++;
	}
	M += TEXT("\n");

	// 产销依赖连线
	for (const auto& DPU : SortedDPUs)
	{
		if (!DPU) continue;
		TArray<FName> ConsumedFacts = DPU->GetConsumedFacts();

		for (const FName& Fact : ConsumedFacts)
		{
			FName* ProducerName = ProducerMap.Find(Fact);
			FString FieldColor = FieldColorMap.FindRef(Fact);

			if (ProducerName)
			{
				M += FString::Printf(TEXT("    %s -->|%s| %s\n"),
					*ProducerName->ToString(), *Fact.ToString(), *DPU->DPUName.ToString());
			}
			else if (InitialFields.Num() > 0)
			{
				M += FString::Printf(TEXT("    DC_Init -->|%s| %s\n"),
					*Fact.ToString(), *DPU->DPUName.ToString());
			}
			else { continue; }

			if (!FieldColor.IsEmpty())
			{
				ColoredLinks.Add({LinkIndex, FieldColor});
			}
			LinkIndex++;
		}
	}

	M += TEXT("\n");

	// linkStyle 着色
	for (const FColoredLink& Link : ColoredLinks)
	{
		M += FString::Printf(TEXT("    linkStyle %d stroke:%s,stroke-width:2px\n"),
			Link.Index, *Link.Color);
	}

	M += TEXT("\n");

	// DC Final 节点
	if (DC)
	{
		FString FinalLines;
		for (const auto& Pair : DC->GetAllFacts())
		{
			if (!DC->GetContextFieldNames().Contains(Pair.Key))
			{
				if (!FinalLines.IsEmpty()) FinalLines += TEXT("<br/>");
				FString ValueStr = DC->GetBool(Pair.Key) ? TEXT("true") : TEXT("false");
				FinalLines += FString::Printf(TEXT("%s = %s"), *Pair.Key.ToString(), *ValueStr);
			}
		}
		if (!FinalLines.IsEmpty())
		{
			M += FString::Printf(TEXT("    DC_Final[\"<b>DC Final</b><br/>%s\"]:::finalCtx\n"), *FinalLines);
		}
		else
		{
			M += TEXT("    DC_Final[\"<b>DC Final</b><br/>(empty)\"]:::finalCtx\n");
		}
	}

	M += TEXT("\n");

	// 保存文件
	FString SafeLabel = Label.Replace(TEXT(" "), TEXT("_")).Replace(TEXT("."), TEXT("_"));
	FString FileName = FString::Printf(TEXT("DPU_DAG_%s_%s.mmd"),
		*SafeLabel, *FDateTime::Now().ToString(TEXT("%Y%m%d_%H%M%S")));
	FString FilePath = FPaths::Combine(FPaths::ProjectSavedDir(), TEXT("Graphs"), FileName);

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
