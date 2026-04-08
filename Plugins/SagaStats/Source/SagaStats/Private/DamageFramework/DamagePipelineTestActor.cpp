// DamagePipelineTestActor.cpp — v4.7: Predicate/Condition 双层 + Operation Set Context
#include "DamageFramework/DamagePipelineTestActor.h"
#include "DamageFramework/DamageContext.h"
#include "DamageFramework/DamageCondition.h"
#include "DamageFramework/DamagePredicate.h"

// 引入所有只狼 DamageRule（每个文件包含 Effect + DamageCondition + Logic）
#include "DamageFramework/Sekiro/DR_Mixup.h"
#include "DamageFramework/Sekiro/DR_Guard.h"
#include "DamageFramework/Sekiro/DR_Death.h"
#include "DamageFramework/Sekiro/DR_Hurt.h"
#include "DamageFramework/Sekiro/DR_Collapse.h"
#include "DamageFramework/Sekiro/DR_Poison.h"
#include "DamageFramework/Sekiro/DR_AttackerBound.h"
#include "DamageFramework/Sekiro/DR_LightningInAir.h"
#include "DamageFramework/Sekiro/DR_LightningOnGround.h"
#include "DamageFramework/Sekiro/DR_Toughness.h"
#include "DamageFramework/Sekiro/DR_SuperArmor.h"
#include "DamageFramework/Sekiro/DR_CollapseJustGuard.h"

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

static UDamagePredicate_Single* MakeCtxCheck(UObject* Outer, FName ContextKey, bool bReverse = false)
{
	UDamageCondition_ContextCheck* Cond = NewObject<UDamageCondition_ContextCheck>(Outer);
	Cond->ContextKey = ContextKey;

	UDamagePredicate_Single* Single = NewObject<UDamagePredicate_Single>(Outer);
	Single->Condition = Cond;
	Single->bReverse = bReverse;
	return Single;
}

static UDamageRule* MakeDamageRule(UObject* Outer, FName Name, TSubclassOf<UDamageOperationBase> OpClass)
{
	UDamageRule* Def = NewObject<UDamageRule>(Outer);
	Def->RuleName = Name;
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
	Pipeline = NewObject<UDamagePipeline>(this);
	Pipeline->PipelineName = FName("SekiroMVPv4.7");
	Pipeline->bAutoExportMermaid = true;

	// ================================================================
	// 第一阶段：创建所有 DamageRule（ProducesEffectType 自动从 OperationClass CDO 获取）
	// ================================================================

	UDamageRule* Mixup = MakeDamageRule(this, "Mixup", UDamageOperation_Mixup::StaticClass());
	UDamageRule* Guard = MakeDamageRule(this, "Guard", UDamageOperation_Guard::StaticClass());
	UDamageRule* Death = MakeDamageRule(this, "Death", UDamageOperation_Death::StaticClass());
	UDamageRule* Collapse = MakeDamageRule(this, "Collapse", UDamageOperation_Collapse::StaticClass());
	UDamageRule* Hurt = MakeDamageRule(this, "Hurt", UDamageOperation_Hurt::StaticClass());
	UDamageRule* CollapseGuard = MakeDamageRule(this, "CollapseGuard", UDamageOperation_CollapseGuard::StaticClass());
	UDamageRule* CollapseJustGuard = MakeDamageRule(this, "CollapseJustGuard", UDamageOperation_CollapseJustGuard::StaticClass());
	UDamageRule* AttackerBound = MakeDamageRule(this, "AttackerBound", UDamageOperation_AttackerBound::StaticClass());
	UDamageRule* Poison = MakeDamageRule(this, "Poison", UDamageOperation_Poison::StaticClass());
	UDamageRule* Toughness = MakeDamageRule(this, "Toughness", UDamageOperation_Toughness::StaticClass());
	UDamageRule* SuperArmor = MakeDamageRule(this, "SuperArmor", UDamageOperation_SuperArmor::StaticClass());
	UDamageRule* LightningOnGround = MakeDamageRule(this, "LightningOnGround", UDamageOperation_LightningOnGround::StaticClass());
	UDamageRule* LightningInAir = MakeDamageRule(this, "LightningInAir", UDamageOperation_LightningInAir::StaticClass());

	// ================================================================
	// 第二阶段：设置 Condition（Predicate/Condition 双层）
	// ================================================================

	Guard->Condition = MakeAnd(this, {
		MakeDamageCondition<UDamageCondition_IsGuard>(this),
		MakeDamageCondition<UDamageCondition_LightningInAir>(this, true)
	});

	auto MakeHurtCondition = [this]() -> UDamagePredicate*
	{
		return MakeAnd(this, {
			MakeAnd(this, {
				MakeDamageCondition<UDamageCondition_IsGuard>(this),
				MakeDamageCondition<UDamageCondition_GuardSuccess>(this)
			}, true),
			MakeDamageCondition<UDamageCondition_LightningInAir>(this, true)
		});
	};

	Death->Condition = MakeHurtCondition();
	Collapse->Condition = MakeHurtCondition();
	Hurt->Condition = MakeHurtCondition();
	Toughness->Condition = MakeHurtCondition();
	SuperArmor->Condition = MakeHurtCondition();

	CollapseGuard->Condition = MakeAnd(this, {
		MakeDamageCondition<UDamageCondition_GuardSuccess>(this),
		MakeDamageCondition<UDamageCondition_GuardIsJustGuard>(this, true),
		MakeDamageCondition<UDamageCondition_LightningInAir>(this, true)
	});

	CollapseJustGuard->Condition = MakeAnd(this, {
		MakeDamageCondition<UDamageCondition_GuardSuccess>(this),
		MakeDamageCondition<UDamageCondition_GuardIsJustGuard>(this),
		MakeDamageCondition<UDamageCondition_LightningInAir>(this, true)
	});

	AttackerBound->Condition = MakeAnd(this, {
		MakeDamageCondition<UDamageCondition_GuardSuccess>(this),
		MakeDamageCondition<UDamageCondition_GuardIsJustGuard>(this),
		MakeDamageCondition<UDamageCondition_LightningInAir>(this, true)
	});

	Poison->Condition = MakeDamageCondition<UDamageCondition_GuardIsJustGuard>(this, true);

	LightningOnGround->Condition = MakeAnd(this, {
		MakeCtxCheck(this, FName("Lightning")),
		MakeCtxCheck(this, FName("IsInAir"), true)
	});

	LightningInAir->Condition = MakeAnd(this, {
		MakeCtxCheck(this, FName("Lightning")),
		MakeCtxCheck(this, FName("IsInAir"))
	});

	// ================================================================
	// Build
	// ================================================================

	Pipeline->DamageRules = {
		Mixup, Guard, Death, Collapse, Hurt,
		CollapseGuard, CollapseJustGuard, AttackerBound,
		Poison, Toughness, SuperArmor,
		LightningOnGround, LightningInAir
	};

	FPipelineSortResult SortResult = Pipeline->Build();

	PrintToScreen(FString::Printf(TEXT("=== Sekiro MVP v4.7 Pipeline Built: %d Rules ==="), Pipeline->DamageRules.Num()), FColor::Cyan);

	FString SortOrder;
	for (int32 i = 0; i < SortResult.SortedRules.Num(); i++)
	{
		if (i > 0) SortOrder += TEXT(" -> ");
		SortOrder += SortResult.SortedRules[i]->RuleName.ToString();
	}
	PrintToScreen(FString::Printf(TEXT("Topo Sort: %s"), *SortOrder), FColor::Yellow);
}

void ADamagePipelineTestActor::RunAllScenarios()
{
	PrintToScreen(TEXT("========================================"), FColor::Cyan);
	PrintToScreen(TEXT("  Running All 5 Sekiro Test Scenarios"), FColor::Cyan);
	PrintToScreen(TEXT("========================================"), FColor::Cyan);

	RunScenario_NormalHit();
	RunScenario_Guard();
	RunScenario_JustGuard();
	RunScenario_LightningGround();
	RunScenario_LightningInAir();
}

void ADamagePipelineTestActor::RunScenario(int32 ScenarioIndex)
{
	switch (ScenarioIndex)
	{
	case 1: RunScenario_NormalHit(); break;
	case 2: RunScenario_Guard(); break;
	case 3: RunScenario_JustGuard(); break;
	case 4: RunScenario_LightningGround(); break;
	case 5: RunScenario_LightningInAir(); break;
	default:
		PrintToScreen(FString::Printf(TEXT("Invalid scenario: %d (use 1-5)"), ScenarioIndex), FColor::Red);
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
	Context->SetFloat(FName("DmgLevel"), 3.0f);
	Context->SetFloat(FName("CurrentHP"), 100.0f);

	TArray<FRuleExecutionEntry> Log = Pipeline->Execute(Context);
	PrintScenarioResult(TEXT("1. Normal Slash Hit"), Context, Log);
}

void ADamagePipelineTestActor::RunScenario_Guard()
{
	Pipeline->ScenarioLabel = TEXT("2.Guard");
	UDamageContext* Context = NewObject<UDamageContext>(this);
	Context->SetFloat(FName("DmgLevel"), 3.0f);
	Context->SetFloat(FName("guard_level"), 3.0f);
	Context->SetFloat(FName("CurrentHP"), 100.0f);

	TArray<FRuleExecutionEntry> Log = Pipeline->Execute(Context);
	PrintScenarioResult(TEXT("2. Guard (non-JustGuard)"), Context, Log);
}

void ADamagePipelineTestActor::RunScenario_JustGuard()
{
	Pipeline->ScenarioLabel = TEXT("3.JustGuard");
	UDamageContext* Context = NewObject<UDamageContext>(this);
	Context->SetFloat(FName("DmgLevel"), 3.0f);
	Context->SetFloat(FName("guard_level"), 5.0f);
	Context->SetFloat(FName("CurrentHP"), 100.0f);
	Context->SetBool(FName("IsPlayer"), true);

	TArray<FRuleExecutionEntry> Log = Pipeline->Execute(Context);
	PrintScenarioResult(TEXT("3. JustGuard (Deflect)"), Context, Log);
}

void ADamagePipelineTestActor::RunScenario_LightningGround()
{
	Pipeline->ScenarioLabel = TEXT("4.LightningGround");
	UDamageContext* Context = NewObject<UDamageContext>(this);
	Context->SetBool(FName("Lightning"), true);
	Context->SetBool(FName("IsInAir"), false);
	Context->SetFloat(FName("DmgLevel"), 3.0f);
	Context->SetFloat(FName("CurrentHP"), 100.0f);

	TArray<FRuleExecutionEntry> Log = Pipeline->Execute(Context);
	PrintScenarioResult(TEXT("4. Lightning on Ground"), Context, Log);
}

void ADamagePipelineTestActor::RunScenario_LightningInAir()
{
	Pipeline->ScenarioLabel = TEXT("5.LightningInAir");
	UDamageContext* Context = NewObject<UDamageContext>(this);
	Context->SetBool(FName("Lightning"), true);
	Context->SetBool(FName("IsInAir"), true);
	Context->SetFloat(FName("DmgLevel"), 3.0f);
	Context->SetFloat(FName("CurrentHP"), 100.0f);

	TArray<FRuleExecutionEntry> Log = Pipeline->Execute(Context);
	PrintScenarioResult(TEXT("5. Lightning in Air"), Context, Log);
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
	for (const auto& Pair : Context->GetAllDamageEffects())
	{
		FString TypeName = Pair.Key ? Pair.Key->GetName() : TEXT("null");
		PrintToScreen(FString::Printf(TEXT("    [%s]"), *TypeName), FColor::Green);
	}
}
