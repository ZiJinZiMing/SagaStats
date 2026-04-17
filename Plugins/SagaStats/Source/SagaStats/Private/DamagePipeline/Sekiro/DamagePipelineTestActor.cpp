/***************************************************************************************************************
* Plugin:       SagaStats
* Author:       Jinming Zhang
* Description:  SagaStats offers modular damage process and meter systems to support adaptable status management
****************************************************************************************************************/

// Sekiro/DamagePipelineTestActor.cpp — 只狼最小验证 Actor（5 Rule × 3 Scenario）
//
// 保留的 5 个 Rule 作为 C++ 测试单元：Mixup / Guard / Hurt / Collapse / CollapseJustGuard
// 3 个场景覆盖：普通命中 / 格挡 / 完美格挡
#include "DamagePipeline/Sekiro/DamagePipelineTestActor.h"
#include "DamagePipeline/DamageContext.h"
#include "DamagePipeline/DamageCondition.h"
#include "DamagePipeline/DamageCondition_Effect.h"
#include "DamagePipeline/DamagePredicate.h"
#include "DamagePipeline/DamagePipelineResults.h"

#include "DamagePipeline/Sekiro/SekiroAttackContext.h"
#include "DamagePipeline/Sekiro/DR_Mixup.h"
#include "DamagePipeline/Sekiro/DR_Guard.h"
#include "DamagePipeline/Sekiro/DR_Hurt.h"
#include "DamagePipeline/Sekiro/DR_Collapse.h"
#include "DamagePipeline/Sekiro/DR_CollapseJustGuard.h"

#include "SagaStatsLog.h"
#include "Kismet/KismetSystemLibrary.h"

// ============================================================================
// 辅助函数
// ============================================================================

static UDamagePredicate_And* MakeAnd(UObject* Outer, const TArray<UDamagePredicate*>& Children, bool bReverse = false)
{
	UDamagePredicate_And* Node = NewObject<UDamagePredicate_And>(Outer);
	for (UDamagePredicate* Child : Children) Node->Predicates.Add(Child);
	Node->bReverse = bReverse;
	return Node;
}

template<typename TCondClass>
static UDamagePredicate_Single* MakeDamageCondition(UObject* Outer, bool bReverse = false)
{
	TCondClass* Cond = NewObject<TCondClass>(Outer);

	UDamagePredicate_Single* Single = NewObject<UDamagePredicate_Single>(Outer);
	Single->Condition = Cond;
	Single->bReverse = bReverse;
	return Single;
}

static UDamageRule* MakeDamageRule(UObject* Outer, FName Name, TSubclassOf<UDamageOperationBase> OpClass)
{
	// 以 Name 作为 UObject 名字，让 Rule->GetFName() 返回它
	UDamageRule* Def = NewObject<UDamageRule>(Outer, Name);
	Def->OperationClass = OpClass;
	return Def;
}

// ============================================================================
// ADamagePipelineTestActor
// ============================================================================

ADamagePipelineTestActor::ADamagePipelineTestActor()
{
	PrimaryActorTick.bCanEverTick = false;
}

void ADamagePipelineTestActor::BeginPlay()
{
	Super::BeginPlay();
	BuildSekiroPipeline();

	if (bAutoRunOnBeginPlay)
	{
		RunAllScenarios();
	}
}

void ADamagePipelineTestActor::BuildSekiroPipeline()
{
	Pipeline = NewObject<UDamagePipeline>(this, FName("SekiroMVPPipeline"));
	Pipeline->bAutoExportMermaid = true;

	// ------------------------------------------------------------------
	// 5 个 Rule
	// ------------------------------------------------------------------
	UDamageRule* Mixup             = MakeDamageRule(this, "Mixup",             UDamageOperation_Mixup::StaticClass());
	UDamageRule* Guard             = MakeDamageRule(this, "Guard",             UDamageOperation_Guard::StaticClass());
	UDamageRule* Hurt              = MakeDamageRule(this, "Hurt",              UDamageOperation_Hurt::StaticClass());
	UDamageRule* Collapse          = MakeDamageRule(this, "Collapse",          UDamageOperation_Collapse::StaticClass());
	UDamageRule* CollapseJustGuard = MakeDamageRule(this, "CollapseJustGuard", UDamageOperation_CollapseJustGuard::StaticClass());

	// ------------------------------------------------------------------
	// Condition 装配
	// ------------------------------------------------------------------

	// Mixup: 无 Condition（始终触发，负责生产 FMixupEffect 供下游消费）

	// Guard: 攻击是 Guard 类型（消费 FMixupEffect.bIsGuard）
	Guard->Condition = MakeDamageCondition<UDamageCondition_IsGuard>(this);

	// Hurt / Collapse: 不是 "Guard 成功" —— NOT(IsGuard AND GuardSuccess)
	auto MakeNotGuardSuccessCondition = [this]() -> UDamagePredicate*
	{
		return MakeAnd(this, {
			MakeDamageCondition<UDamageCondition_IsGuard>(this),
			MakeDamageCondition<UDamageCondition_GuardSuccess>(this)
		}, /*bReverse=*/true);
	};
	Hurt->Condition     = MakeNotGuardSuccessCondition();
	Collapse->Condition = MakeNotGuardSuccessCondition();

	// CollapseJustGuard: Guard 成功 且 是完美格挡
	CollapseJustGuard->Condition = MakeAnd(this, {
		MakeDamageCondition<UDamageCondition_GuardSuccess>(this),
		MakeDamageCondition<UDamageCondition_GuardIsJustGuard>(this)
	});

	// ------------------------------------------------------------------
	// Build
	// ------------------------------------------------------------------
	Pipeline->DamageRules = { Mixup, Guard, Hurt, Collapse, CollapseJustGuard };

	FPipelineSortResult SortResult = Pipeline->Build();

	PrintToScreen(FString::Printf(TEXT("=== Sekiro MVP Pipeline Built: %d Rules ==="), Pipeline->DamageRules.Num()), FColor::Cyan);

	FString SortOrder;
	for (int32 i = 0; i < SortResult.SortedRules.Num(); i++)
	{
		if (i > 0) SortOrder += TEXT(" -> ");
		SortOrder += SortResult.SortedRules[i]->GetName();
	}
	PrintToScreen(FString::Printf(TEXT("Topo Sort: %s"), *SortOrder), FColor::Yellow);
}

void ADamagePipelineTestActor::RunAllScenarios()
{
	PrintToScreen(TEXT("========================================"), FColor::Cyan);
	PrintToScreen(TEXT("  Running All 3 Sekiro Test Scenarios"), FColor::Cyan);
	PrintToScreen(TEXT("========================================"), FColor::Cyan);

	RunScenario_NormalHit();
	RunScenario_Guard();
	RunScenario_JustGuard();
}

void ADamagePipelineTestActor::RunScenario(int32 ScenarioIndex)
{
	switch (ScenarioIndex)
	{
	case 1: RunScenario_NormalHit(); break;
	case 2: RunScenario_Guard(); break;
	case 3: RunScenario_JustGuard(); break;
	default:
		PrintToScreen(FString::Printf(TEXT("Invalid scenario: %d (use 1-3)"), ScenarioIndex), FColor::Red);
		break;
	}
}

// ============================================================================
// 场景实现
// ============================================================================

void ADamagePipelineTestActor::RunScenario_NormalHit()
{
	Pipeline->ScenarioLabel = TEXT("1.NormalSlashHit");
	UDamageContext* Context = NewObject<UDamageContext>(this);
	FSekiroAttackContext Atk;
	Atk.DmgLevel = 3.0f;
	Atk.CurrentHP = 100.0f;
	UDamagePipelineResults::WriteEffect<FSekiroAttackContext>(Context, Atk);

	TArray<FRuleExecutionEntry> Log = Pipeline->Execute(Context);
	PrintScenarioResult(TEXT("1. Normal Slash Hit"), Context, Log);
}

void ADamagePipelineTestActor::RunScenario_Guard()
{
	Pipeline->ScenarioLabel = TEXT("2.Guard");
	UDamageContext* Context = NewObject<UDamageContext>(this);
	FSekiroAttackContext Atk;
	Atk.DmgLevel = 3.0f;
	Atk.GuardLevel = 3.0f;
	Atk.CurrentHP = 100.0f;
	UDamagePipelineResults::WriteEffect<FSekiroAttackContext>(Context, Atk);

	TArray<FRuleExecutionEntry> Log = Pipeline->Execute(Context);
	PrintScenarioResult(TEXT("2. Guard (non-JustGuard)"), Context, Log);
}

void ADamagePipelineTestActor::RunScenario_JustGuard()
{
	Pipeline->ScenarioLabel = TEXT("3.JustGuard");
	UDamageContext* Context = NewObject<UDamageContext>(this);
	FSekiroAttackContext Atk;
	Atk.DmgLevel = 3.0f;
	Atk.GuardLevel = 5.0f;
	Atk.CurrentHP = 100.0f;
	Atk.bIsPlayer = true;
	UDamagePipelineResults::WriteEffect<FSekiroAttackContext>(Context, Atk);

	TArray<FRuleExecutionEntry> Log = Pipeline->Execute(Context);
	PrintScenarioResult(TEXT("3. JustGuard (Deflect)"), Context, Log);
}

// ============================================================================
// 输出
// ============================================================================

void ADamagePipelineTestActor::PrintToScreen(const FString& Message, FColor Color) const
{
	UKismetSystemLibrary::PrintString(this, Message, true, true, Color, ScreenMessageDuration);
}

void ADamagePipelineTestActor::PrintScenarioResult(
	const FString& ScenarioName, UDamageContext* Context,
	const TArray<FRuleExecutionEntry>& Log) const
{
	PrintToScreen(TEXT(""), FColor::White);
	PrintToScreen(FString::Printf(TEXT("--- %s ---"), *ScenarioName), FColor::Green);

	for (const FRuleExecutionEntry& Entry : Log)
	{
		FString Status = Entry.bExecuted ? TEXT("[EXEC]") : TEXT("[SKIP]");
		FColor Color = Entry.bExecuted ? FColor::White : FColor::Silver;
		PrintToScreen(FString::Printf(TEXT("  %s %s"), *Status, *Entry.RuleName.ToString()), Color);
	}

	PrintToScreen(TEXT("  Context DamageEffects:"), FColor::Yellow);
	for (const auto& Pair : UDamagePipelineResults::GetAllEffects(Context))
	{
		FString TypeName = Pair.Key ? Pair.Key->GetName() : TEXT("null");
		PrintToScreen(FString::Printf(TEXT("    [%s]"), *TypeName), FColor::Green);
	}
}
