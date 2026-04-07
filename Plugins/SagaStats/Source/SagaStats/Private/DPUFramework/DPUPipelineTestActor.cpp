// DPUPipelineTestActor.cpp — v4.7: Predicate/Condition 双层 + Logic Set DC
#include "DPUFramework/DPUPipelineTestActor.h"
#include "DPUFramework/DamageContext.h"
#include "DPUFramework/DPUCondition.h"
#include "DPUFramework/DPUPredicate.h"

// 引入所有只狼 DPU（每个文件包含 Fact + DPUCondition + Logic）
#include "DPUFramework/Sekiro/DPU_Mixup.h"
#include "DPUFramework/Sekiro/DPU_Guard.h"
#include "DPUFramework/Sekiro/DPU_Death.h"
#include "DPUFramework/Sekiro/DPU_Hurt.h"
#include "DPUFramework/Sekiro/DPU_Collapse.h"
#include "DPUFramework/Sekiro/DPU_Poison.h"
#include "DPUFramework/Sekiro/DPU_AttackerBound.h"
#include "DPUFramework/Sekiro/DPU_LightningInAir.h"
#include "DPUFramework/Sekiro/DPU_LightningOnGround.h"
#include "DPUFramework/Sekiro/DPU_Toughness.h"
#include "DPUFramework/Sekiro/DPU_SuperArmor.h"
#include "DPUFramework/Sekiro/DPU_CollapseJustGuard.h"

#include "SagaStatsLog.h"
#include "Kismet/KismetSystemLibrary.h"

// ============================================================================
// 辅助函数
// ============================================================================

static UDPUPredicate_And* MakeAnd(UObject* Outer, const TArray<UDPUPredicate*>& Children, bool bReverse = false)
{
	UDPUPredicate_And* Node = NewObject<UDPUPredicate_And>(Outer);
	for (UDPUPredicate* Child : Children) Node->Predicates.Add(Child);
	Node->bReverse = bReverse;
	return Node;
}

template<typename TCondClass>
static UDPUPredicate_Single* MakeDPUCondition(UObject* Outer, bool bReverse = false)
{
	TCondClass* Cond = NewObject<TCondClass>(Outer);

	UDPUPredicate_Single* Single = NewObject<UDPUPredicate_Single>(Outer);
	Single->Condition = Cond;
	Single->bReverse = bReverse;
	return Single;
}

static UDPUPredicate_Single* MakeCtxCheck(UObject* Outer, FName ContextKey, bool bReverse = false)
{
	UDPUCondition_ContextCheck* Cond = NewObject<UDPUCondition_ContextCheck>(Outer);
	Cond->ContextKey = ContextKey;

	UDPUPredicate_Single* Single = NewObject<UDPUPredicate_Single>(Outer);
	Single->Condition = Cond;
	Single->bReverse = bReverse;
	return Single;
}

static UDPUDefinition* MakeDPU(UObject* Outer, FName Name, TSubclassOf<UDPULogicBase> Logic)
{
	UDPUDefinition* Def = NewObject<UDPUDefinition>(Outer);
	Def->DPUName = Name;
	Def->LogicClass = Logic;
	return Def;
}

// ============================================================================
// ADPUPipelineTestActor
// ============================================================================

ADPUPipelineTestActor::ADPUPipelineTestActor()
{
	PrimaryActorTick.bCanEverTick = false;
}

void ADPUPipelineTestActor::BeginPlay()
{
	Super::BeginPlay();
	BuildSekiroPipeline();

	if (bAutoRunOnBeginPlay)
	{
		RunAllScenarios();
	}
}

void ADPUPipelineTestActor::BuildSekiroPipeline()
{
	Pipeline = NewObject<UPipelineAsset>(this);
	Pipeline->PipelineName = FName("SekiroMVPv4.7");
	Pipeline->bAutoExportMermaid = true;

	// ================================================================
	// 第一阶段：创建所有 DPU（ProducesFactType 自动从 LogicClass CDO 获取）
	// ================================================================

	UDPUDefinition* Mixup = MakeDPU(this, "Mixup", UDPULogic_Mixup::StaticClass());
	UDPUDefinition* Guard = MakeDPU(this, "Guard", UDPULogic_Guard::StaticClass());
	UDPUDefinition* Death = MakeDPU(this, "Death", UDPULogic_Death::StaticClass());
	UDPUDefinition* Collapse = MakeDPU(this, "Collapse", UDPULogic_Collapse::StaticClass());
	UDPUDefinition* Hurt = MakeDPU(this, "Hurt", UDPULogic_Hurt::StaticClass());
	UDPUDefinition* CollapseGuard = MakeDPU(this, "CollapseGuard", UDPULogic_CollapseGuard::StaticClass());
	UDPUDefinition* CollapseJustGuard = MakeDPU(this, "CollapseJustGuard", UDPULogic_CollapseJustGuard::StaticClass());
	UDPUDefinition* AttackerBound = MakeDPU(this, "AttackerBound", UDPULogic_AttackerBound::StaticClass());
	UDPUDefinition* Poison = MakeDPU(this, "Poison", UDPULogic_Poison::StaticClass());
	UDPUDefinition* Toughness = MakeDPU(this, "Toughness", UDPULogic_Toughness::StaticClass());
	UDPUDefinition* SuperArmor = MakeDPU(this, "SuperArmor", UDPULogic_SuperArmor::StaticClass());
	UDPUDefinition* LightningOnGround = MakeDPU(this, "LightningOnGround", UDPULogic_LightningOnGround::StaticClass());
	UDPUDefinition* LightningInAir = MakeDPU(this, "LightningInAir", UDPULogic_LightningInAir::StaticClass());

	// ================================================================
	// 第二阶段：设置 Condition（Predicate/Condition 双层）
	// ================================================================

	Guard->Condition = MakeAnd(this, {
		MakeDPUCondition<UDPUCondition_IsGuard>(this),
		MakeDPUCondition<UDPUCondition_LightningInAir>(this, true)
	});

	auto MakeHurtCondition = [this]() -> UDPUPredicate*
	{
		return MakeAnd(this, {
			MakeAnd(this, {
				MakeDPUCondition<UDPUCondition_IsGuard>(this),
				MakeDPUCondition<UDPUCondition_GuardSuccess>(this)
			}, true),
			MakeDPUCondition<UDPUCondition_LightningInAir>(this, true)
		});
	};

	Death->Condition = MakeHurtCondition();
	Collapse->Condition = MakeHurtCondition();
	Hurt->Condition = MakeHurtCondition();
	Toughness->Condition = MakeHurtCondition();
	SuperArmor->Condition = MakeHurtCondition();

	CollapseGuard->Condition = MakeAnd(this, {
		MakeDPUCondition<UDPUCondition_GuardSuccess>(this),
		MakeDPUCondition<UDPUCondition_GuardIsJustGuard>(this, true),
		MakeDPUCondition<UDPUCondition_LightningInAir>(this, true)
	});

	CollapseJustGuard->Condition = MakeAnd(this, {
		MakeDPUCondition<UDPUCondition_GuardSuccess>(this),
		MakeDPUCondition<UDPUCondition_GuardIsJustGuard>(this),
		MakeDPUCondition<UDPUCondition_LightningInAir>(this, true)
	});

	AttackerBound->Condition = MakeAnd(this, {
		MakeDPUCondition<UDPUCondition_GuardSuccess>(this),
		MakeDPUCondition<UDPUCondition_GuardIsJustGuard>(this),
		MakeDPUCondition<UDPUCondition_LightningInAir>(this, true)
	});

	Poison->Condition = MakeDPUCondition<UDPUCondition_GuardIsJustGuard>(this, true);

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

	Pipeline->DPUDefinitions = {
		Mixup, Guard, Death, Collapse, Hurt,
		CollapseGuard, CollapseJustGuard, AttackerBound,
		Poison, Toughness, SuperArmor,
		LightningOnGround, LightningInAir
	};

	FPipelineSortResult SortResult = Pipeline->Build();

	PrintToScreen(FString::Printf(TEXT("=== Sekiro MVP v4.7 Pipeline Built: %d DPUs ==="), Pipeline->DPUDefinitions.Num()), FColor::Cyan);

	FString SortOrder;
	for (int32 i = 0; i < SortResult.SortedDPUs.Num(); i++)
	{
		if (i > 0) SortOrder += TEXT(" -> ");
		SortOrder += SortResult.SortedDPUs[i]->DPUName.ToString();
	}
	PrintToScreen(FString::Printf(TEXT("Topo Sort: %s"), *SortOrder), FColor::Yellow);
}

void ADPUPipelineTestActor::RunAllScenarios()
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

void ADPUPipelineTestActor::RunScenario(int32 ScenarioIndex)
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

void ADPUPipelineTestActor::RunScenario_NormalHit()
{
	Pipeline->ScenarioLabel = TEXT("1.NormalSlashHit");
	UDamageContext* DC = NewObject<UDamageContext>(this);
	DC->SetFloat(FName("DmgLevel"), 3.0f);
	DC->SetFloat(FName("CurrentHP"), 100.0f);

	TArray<FDPUExecutionEntry> Log = Pipeline->Execute(DC);
	PrintScenarioResult(TEXT("1. Normal Slash Hit"), DC, Log);
}

void ADPUPipelineTestActor::RunScenario_Guard()
{
	Pipeline->ScenarioLabel = TEXT("2.Guard");
	UDamageContext* DC = NewObject<UDamageContext>(this);
	DC->SetFloat(FName("DmgLevel"), 3.0f);
	DC->SetFloat(FName("guard_level"), 3.0f);
	DC->SetFloat(FName("CurrentHP"), 100.0f);

	TArray<FDPUExecutionEntry> Log = Pipeline->Execute(DC);
	PrintScenarioResult(TEXT("2. Guard (non-JustGuard)"), DC, Log);
}

void ADPUPipelineTestActor::RunScenario_JustGuard()
{
	Pipeline->ScenarioLabel = TEXT("3.JustGuard");
	UDamageContext* DC = NewObject<UDamageContext>(this);
	DC->SetFloat(FName("DmgLevel"), 3.0f);
	DC->SetFloat(FName("guard_level"), 5.0f);
	DC->SetFloat(FName("CurrentHP"), 100.0f);
	DC->SetBool(FName("IsPlayer"), true);

	TArray<FDPUExecutionEntry> Log = Pipeline->Execute(DC);
	PrintScenarioResult(TEXT("3. JustGuard (Deflect)"), DC, Log);
}

void ADPUPipelineTestActor::RunScenario_LightningGround()
{
	Pipeline->ScenarioLabel = TEXT("4.LightningGround");
	UDamageContext* DC = NewObject<UDamageContext>(this);
	DC->SetBool(FName("Lightning"), true);
	DC->SetBool(FName("IsInAir"), false);
	DC->SetFloat(FName("DmgLevel"), 3.0f);
	DC->SetFloat(FName("CurrentHP"), 100.0f);

	TArray<FDPUExecutionEntry> Log = Pipeline->Execute(DC);
	PrintScenarioResult(TEXT("4. Lightning on Ground"), DC, Log);
}

void ADPUPipelineTestActor::RunScenario_LightningInAir()
{
	Pipeline->ScenarioLabel = TEXT("5.LightningInAir");
	UDamageContext* DC = NewObject<UDamageContext>(this);
	DC->SetBool(FName("Lightning"), true);
	DC->SetBool(FName("IsInAir"), true);
	DC->SetFloat(FName("DmgLevel"), 3.0f);
	DC->SetFloat(FName("CurrentHP"), 100.0f);

	TArray<FDPUExecutionEntry> Log = Pipeline->Execute(DC);
	PrintScenarioResult(TEXT("5. Lightning in Air"), DC, Log);
}

// ============================================================================
// 输出
// ============================================================================

void ADPUPipelineTestActor::PrintToScreen(const FString& Message, FColor Color) const
{
	UKismetSystemLibrary::PrintString(this, Message, true, true, Color, ScreenMessageDuration);
}

void ADPUPipelineTestActor::PrintScenarioResult(
	const FString& ScenarioName, UDamageContext* DC,
	const TArray<FDPUExecutionEntry>& Log) const
{
	PrintToScreen(TEXT(""), FColor::White);
	PrintToScreen(FString::Printf(TEXT("--- %s ---"), *ScenarioName), FColor::Green);

	for (const FDPUExecutionEntry& Entry : Log)
	{
		FString Status = Entry.bExecuted ? TEXT("[EXEC]") : TEXT("[SKIP]");
		FColor Color = Entry.bExecuted ? FColor::White : FColor::Silver;
		PrintToScreen(FString::Printf(TEXT("  %s %s"), *Status, *Entry.DPUName.ToString()), Color);
	}

	PrintToScreen(TEXT("  DC DPU Facts:"), FColor::Yellow);
	for (const auto& Pair : DC->GetAllDPUFacts())
	{
		FString TypeName = Pair.Key ? Pair.Key->GetName() : TEXT("null");
		PrintToScreen(FString::Printf(TEXT("    [%s]"), *TypeName), FColor::Green);
	}
}
