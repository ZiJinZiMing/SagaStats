// DamagePipeline.cpp — 自洽的 Pipeline：拓扑排序烘焙 + 执行 + Mermaid DAG 导出
#include "DamageFramework/DamagePipeline.h"
#include "DamageFramework/DamageCondition.h"
#include "SagaStatsLog.h"
#include "Misc/FileHelper.h"
#include "HAL/PlatformFileManager.h"
#include "Misc/Paths.h"

// ============================================================================
// 编辑器：属性变化时置 bIsBaked = false
// ============================================================================

#if WITH_EDITOR
void UDamagePipeline::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	if (bIsBaked)
	{
		bIsBaked = false;
		UE_LOG(LogSagaStats, Log, TEXT("DamagePipeline: 属性已修改，bIsBaked 置为 false，需要重新 Build"));
	}
}

void UDamagePipeline::PostEditChangeChainProperty(FPropertyChangedChainEvent& PropertyChangedEvent)
{
	Super::PostEditChangeChainProperty(PropertyChangedEvent);

	if (bIsBaked)
	{
		bIsBaked = false;
		UE_LOG(LogSagaStats, Log, TEXT("DamagePipeline: 嵌套属性已修改，bIsBaked 置为 false，需要重新 Build"));
	}
}
#endif

// ============================================================================
// 稳定拓扑排序（Kahn 算法 BFS + 原始索引优先队列）
// ============================================================================

FPipelineSortResult UDamagePipeline::StableTopologicalSort(const TArray<UDamageRule*>& Rules)
{
	FPipelineSortResult Result;
	if (Rules.Num() == 0) return Result;

	// 1. 分配原始索引
	TMap<UDamageRule*, int32> OriginalIndex;
	for (int32 i = 0; i < Rules.Num(); i++)
	{
		OriginalIndex.Add(Rules[i], i);
	}

	// 2. 构建生产者映射：EffectType -> DamageRule（v4.8: 类型即 key）
	TMap<UScriptStruct*, UDamageRule*> ProducerMap;
	for (UDamageRule* Rule : Rules)
	{
		if (!Rule) continue;
		UScriptStruct* EffectType = Rule->GetProducesEffectType();
		if (EffectType) ProducerMap.Add(EffectType, Rule);
	}

	// 3. 构建依赖图
	TMap<UDamageRule*, TSet<UDamageRule*>> Dependencies;
	TMap<UDamageRule*, TSet<UDamageRule*>> Dependents;
	TMap<UDamageRule*, int32> InDegree;

	for (UDamageRule* Rule : Rules)
	{
		if (!Rule) continue;
		Dependencies.FindOrAdd(Rule);
		Dependents.FindOrAdd(Rule);
		InDegree.FindOrAdd(Rule) = 0;
	}

	for (UDamageRule* Rule : Rules)
	{
		if (!Rule) continue;
		TArray<UScriptStruct*> ConsumedTypes = Rule->GetConsumedEffectTypes();
		for (UScriptStruct* Type : ConsumedTypes)
		{
			UDamageRule** ProducerPtr = ProducerMap.Find(Type);
			if (ProducerPtr && *ProducerPtr != Rule)
			{
				UDamageRule* Producer = *ProducerPtr;
				if (!Dependencies[Rule].Contains(Producer))
				{
					Dependencies[Rule].Add(Producer);
					Dependents[Producer].Add(Rule);
					InDegree[Rule]++;
				}
			}
		}
	}

	// 4. 稳定 BFS
	TArray<UDamageRule*> ReadyQueue;
	for (UDamageRule* Rule : Rules)
	{
		if (!Rule) continue;
		if (InDegree[Rule] == 0)
		{
			ReadyQueue.Add(Rule);
		}
	}
	ReadyQueue.Sort([&OriginalIndex](const UDamageRule& A, const UDamageRule& B)
	{
		return OriginalIndex[const_cast<UDamageRule*>(&A)] < OriginalIndex[const_cast<UDamageRule*>(&B)];
	});

	TArray<UDamageRule*> Sorted;
	while (ReadyQueue.Num() > 0)
	{
		UDamageRule* Current = ReadyQueue[0];
		ReadyQueue.RemoveAt(0);
		Sorted.Add(Current);

		for (UDamageRule* Dependent : Dependents[Current])
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
	if (Sorted.Num() < Rules.Num())
	{
		Result.bHasCycle = true;
		for (UDamageRule* Rule : Rules)
		{
			if (!Rule) continue;
			if (!Sorted.Contains(Rule))
			{
				FString DepNames;
				for (UDamageRule* Dep : Dependencies[Rule])
				{
					if (!Sorted.Contains(Dep))
					{
						if (!DepNames.IsEmpty()) DepNames += TEXT(", ");
						DepNames += Dep->RuleName.ToString();
					}
				}
				Result.CycleInfo.Add(FString::Printf(TEXT("%s depends on [%s]"),
					*Rule->RuleName.ToString(), *DepNames));
			}
		}
	}

	for (UDamageRule* Rule : Sorted)
	{
		Result.SortedRules.Add(Rule);
	}

	return Result;
}

// ============================================================================
// Build：拓扑排序烘焙
// ============================================================================

// 递归收集 Predicate 树中所有叶子 Condition
static void CollectLeafConditions(const UDamagePredicate* Pred, TArray<const UDamageCondition*>& OutConditions)
{
	if (!Pred) return;

	if (const UDamagePredicate_Single* Single = Cast<UDamagePredicate_Single>(Pred))
	{
		if (Single->Condition) OutConditions.Add(Single->Condition);
	}
	else if (const UDamagePredicate_And* And = Cast<UDamagePredicate_And>(Pred))
	{
		for (const auto& P : And->Predicates) CollectLeafConditions(P, OutConditions);
	}
	else if (const UDamagePredicate_Or* Or = Cast<UDamagePredicate_Or>(Pred))
	{
		for (const auto& P : Or->Predicates) CollectLeafConditions(P, OutConditions);
	}
}

FPipelineSortResult UDamagePipeline::Build()
{
	TArray<UDamageRule*> RawPtrs;
	for (const auto& Def : DamageRules)
	{
		if (Def) RawPtrs.Add(Def.Get());
	}

	// ---- EffectType 校验 ----
	bool bValidationFailed = false;

	for (const auto& Rule : RawPtrs)
	{
		if (!Rule) continue;

		// 校验 Logic 的 ProducesEffectType
		if (!Rule->GetProducesEffectType())
		{
			UE_LOG(LogSagaStats, Error, TEXT("Pipeline Build 校验失败: DamageRule [%s] 的 Logic 未配置 ProducesEffectType"),
				*Rule->RuleName.ToString());
			bValidationFailed = true;
		}

		// 校验 Condition 树中所有叶子的 ConsumedEffectType（ContextCheck 除外）
		if (Rule->Condition)
		{
			TArray<const UDamageCondition*> Leaves;
			CollectLeafConditions(Rule->Condition, Leaves);
			for (const UDamageCondition* Cond : Leaves)
			{
				if (Cond->IsA<UDamageCondition_ContextCheck>()) continue;
				if (!Cond->GetConsumedEffectType())
				{
					UE_LOG(LogSagaStats, Error,
						TEXT("Pipeline Build 校验失败: DamageRule [%s] 的 Condition [%s] 未配置 ConsumedEffectType"),
						*Rule->RuleName.ToString(), *Cond->GetClass()->GetName());
					bValidationFailed = true;
				}
			}
		}
	}

	FPipelineSortResult Result;
	if (bValidationFailed)
	{
		bIsBaked = false;
		Result.bHasCycle = true;
		Result.CycleInfo.Add(TEXT("EffectType 校验失败，请检查上方 Error 日志"));
		return Result;
	}

	// ---- 拓扑排序 ----
	Result = StableTopologicalSort(RawPtrs);

	SortedRules.Empty();
	for (const auto& Rule : Result.SortedRules)
	{
		SortedRules.Add(Rule.Get());
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
		for (const auto& Rule : SortedRules)
		{
			if (!SortOrder.IsEmpty()) SortOrder += TEXT(" -> ");
			SortOrder += Rule->RuleName.ToString();
		}
		UE_LOG(LogSagaStats, Log, TEXT("Pipeline Build 完成: %s"), *SortOrder);
	}

	return Result;
}

// ============================================================================
// Execute：管线执行
// ============================================================================

TArray<FRuleExecutionEntry> UDamagePipeline::Execute(UDamageContext* Context)
{
	TArray<FRuleExecutionEntry> ExecutionLog;

	if (!bIsBaked)
	{
		Build();
	}

	if (!bIsBaked)
	{
		UE_LOG(LogSagaStats, Error, TEXT("Pipeline 未烘焙（可能有循环依赖），无法执行"));
		return ExecutionLog;
	}

	for (UDamageRule* Rule : SortedRules)
	{
		if (!Rule) continue;

		FRuleExecutionEntry Entry;
		Entry.RuleName = Rule->RuleName;

		// 评估 Predicate（调用 EvaluatePredicate 以应用 bReverse）
		if (Rule->Condition)
		{
			if (!Rule->Condition->EvaluatePredicate(Context))
			{
				Entry.bExecuted = false;
				ExecutionLog.Add(Entry);
				UE_LOG(LogSagaStats, Log, TEXT("  [SKIP] %s"), *Rule->RuleName.ToString());
				continue;
			}
		}

		// 执行逻辑：框架创建 OutEffect → Logic 填字段 → 写入 DC
		if (Rule->OperationClass)
		{
			UDamageOperationBase* Logic = GetOrCreateOperation(Rule->OperationClass);
			UScriptStruct* EffectType = Rule->GetProducesEffectType();
			if (Logic && EffectType)
			{
				FInstancedStruct OutEffect(EffectType);
				Logic->Execute(Context, OutEffect);

				// 校验 OutEffect 类型与声明的 ProducesEffectType 一致
				if (OutEffect.IsValid() && OutEffect.GetScriptStruct() == EffectType)
				{
					Context->SetEffectByType(EffectType, OutEffect);
				}
				else
				{
					UE_LOG(LogSagaStats, Error,
						TEXT("DamageRule %s: OutEffect 类型不匹配！期望 %s，实际 %s"),
						*Rule->RuleName.ToString(),
						*EffectType->GetName(),
						OutEffect.IsValid() ? *OutEffect.GetScriptStruct()->GetName() : TEXT("invalid"));
				}
			}
		}

		Entry.bExecuted = true;
		ExecutionLog.Add(Entry);
		UE_LOG(LogSagaStats, Log, TEXT("  [EXEC] %s"), *Rule->RuleName.ToString());
	}

	UE_LOG(LogSagaStats, Log, TEXT("%s"), *Context->DumpToString());

	if (bAutoExportMermaid)
	{
		ExportMermaidDAG(ExecutionLog, Context);
	}

	return ExecutionLog;
}

UDamageOperationBase* UDamagePipeline::GetOrCreateOperation(TSubclassOf<UDamageOperationBase> OperationClass)
{
	if (!OperationClass) return nullptr;

	UClass* ClassPtr = OperationClass.Get();
	if (TObjectPtr<UDamageOperationBase>* Found = OperationInstances.Find(ClassPtr))
	{
		return Found->Get();
	}

	UDamageOperationBase* NewLogic = NewObject<UDamageOperationBase>(this, ClassPtr);
	OperationInstances.Add(ClassPtr, NewLogic);
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

void UDamagePipeline::ExportMermaidDAG(
	const TArray<FRuleExecutionEntry>& ExecutionLog,
	const UDamageContext* Context) const
{
	// 执行状态映射
	TMap<FName, bool> ExecStatusMap;
	for (const FRuleExecutionEntry& Entry : ExecutionLog)
	{
		ExecStatusMap.Add(Entry.RuleName, Entry.bExecuted);
	}

	// DamageRule 间依赖的颜色分配（以 DamageRule Name 为单位着色）
	TMap<FName, FString> RuleColorMap;
	{
		TArray<FName> OrderedRules;
		for (const auto& Rule : SortedRules)
		{
			if (!Rule) continue;
			if (!RuleColorMap.Contains(Rule->RuleName))
			{
				RuleColorMap.Add(Rule->RuleName, FieldColorPalette[OrderedRules.Num() % FieldColorPaletteSize]);
				OrderedRules.Add(Rule->RuleName);
			}
		}
	}

	// DC 初始字段（事件上下文）
	TArray<TPair<FName, FString>> InitialFields;
	if (Context)
	{
		for (const auto& Pair : Context->GetAllContextFacts())
		{
			FString ValueStr;
			bool bVal = Context->GetBool(Pair.Key);
			float fVal = Context->GetFloat(Pair.Key);
			if (fVal != 0.f)
				ValueStr = FString::SanitizeFloat(fVal);
			else
				ValueStr = bVal ? TEXT("true") : TEXT("false");
			InitialFields.Add({Pair.Key, ValueStr});
		}
	}

	// v4.8: EffectType→DPU 映射用于依赖连线
	TMap<UScriptStruct*, const UDamageRule*> EffectTypeToProducer;
	for (const auto& Rule : SortedRules)
	{
		if (!Rule) continue;
		UScriptStruct* FT = Rule->GetProducesEffectType();
		if (FT) EffectTypeToProducer.Add(FT, Rule);
	}

	// ---- 生成 Mermaid ----
	FString M;
	M += TEXT("graph LR\n");

	FString Label = ScenarioLabel.IsEmpty() ? TEXT("Damage Pipeline") : ScenarioLabel;
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

	// DamageRule 节点
	for (int32 i = 0; i < SortedRules.Num(); i++)
	{
		const UDamageRule* Rule = SortedRules[i];
		if (!Rule) continue;

		const bool* bExec = ExecStatusMap.Find(Rule->RuleName);
		bool bExecuted = bExec ? *bExec : false;
		FString StyleClass = bExecuted ? TEXT("exec") : TEXT("skip");

		// Condition 文本：使用条件树的 GetDisplayString()
		FString CondText = TEXT("(none)");
		if (Rule->Condition)
		{
			CondText = Rule->Condition->GetDisplayString();
		}
		CondText.ReplaceInline(TEXT("&&"), TEXT(" #amp;#amp; "));
		CondText.ReplaceInline(TEXT("||"), TEXT(" #124;#124; "));
		CondText.ReplaceInline(TEXT("\""), TEXT("#quot;"));

		// v4.5: Produces 显示 Effect 类型名
		FString ProducesText;
		if (Rule->GetProducesEffectType())
		{
			FString TypeColor = RuleColorMap.FindRef(Rule->RuleName);
			if (!TypeColor.IsEmpty())
			{
				ProducesText = FString::Printf(TEXT("<font color='%s'>#9632;</font> %s"),
					*TypeColor, *Rule->GetProducesEffectType()->GetName());
			}
			else
			{
				ProducesText = Rule->GetProducesEffectType()->GetName();
			}
		}
		else
		{
			ProducesText = TEXT("(none)");
		}

		M += FString::Printf(
			TEXT("    %s[\"%s #%d<br/>Cond: %s<br/>Produces:<br/>%s\"]:::%s\n"),
			*Rule->RuleName.ToString(), *Rule->RuleName.ToString(), i,
			*CondText, *ProducesText, *StyleClass);
	}

	M += TEXT("\n");

	// 连线
	int32 LinkIndex = 0;
	struct FColoredLink { int32 Index; FString Color; };
	TArray<FColoredLink> ColoredLinks;

	// 隐藏执行顺序链
	if (InitialFields.Num() > 0 && SortedRules.Num() > 0)
	{
		M += FString::Printf(TEXT("    DC_Init ~~~ %s\n"), *SortedRules[0]->RuleName.ToString());
		LinkIndex++;
	}
	for (int32 i = 0; i + 1 < SortedRules.Num(); i++)
	{
		M += FString::Printf(TEXT("    %s ~~~ %s\n"),
			*SortedRules[i]->RuleName.ToString(), *SortedRules[i + 1]->RuleName.ToString());
		LinkIndex++;
	}
	if (SortedRules.Num() > 0)
	{
		M += FString::Printf(TEXT("    %s ~~~ DC_Final\n"), *SortedRules.Last()->RuleName.ToString());
		LinkIndex++;
	}
	M += TEXT("\n");

	// 产销依赖连线（v4.8: EffectType 匹配）
	for (const auto& Rule : SortedRules)
	{
		if (!Rule) continue;
		TArray<UScriptStruct*> ConsumedTypes = Rule->GetConsumedEffectTypes();

		for (UScriptStruct* Type : ConsumedTypes)
		{
			const UDamageRule** ProducerDef = EffectTypeToProducer.Find(Type);
			FString TypeName = Type ? Type->GetName() : TEXT("?");

			if (ProducerDef && *ProducerDef)
			{
				FString ProducerName = (*ProducerDef)->RuleName.ToString();
				FString FieldColor = RuleColorMap.FindRef((*ProducerDef)->RuleName);

				M += FString::Printf(TEXT("    %s -->|%s| %s\n"),
					*ProducerName, *TypeName, *Rule->RuleName.ToString());

				if (!FieldColor.IsEmpty())
				{
					ColoredLinks.Add({LinkIndex, FieldColor});
				}
			}
			else if (InitialFields.Num() > 0)
			{
				M += FString::Printf(TEXT("    DC_Init -->|%s| %s\n"),
					*TypeName, *Rule->RuleName.ToString());
			}
			else { continue; }

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

	// DC Final 节点（DamageRule 产出的 Fact）
	if (Context)
	{
		FString FinalLines;
		for (const auto& Pair : Context->GetAllDamageEffects())
		{
			if (!FinalLines.IsEmpty()) FinalLines += TEXT("<br/>");
			FString TypeName = Pair.Key ? Pair.Key->GetName() : TEXT("null");
			FinalLines += FString::Printf(TEXT("[%s]"), *TypeName);
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
	FString FileName = FString::Printf(TEXT("DR_DAG_%s_%s.mmd"),
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
