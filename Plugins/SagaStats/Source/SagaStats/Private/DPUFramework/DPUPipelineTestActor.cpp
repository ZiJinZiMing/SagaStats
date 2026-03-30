// DPUPipelineTestActor.cpp — v4.4 Fact 版：真正的 Fact 结构体 + 领域方法 + 条件树
#include "DPUFramework/DPUPipelineTestActor.h"
#include "DPUFramework/DamageContext.h"
#include "DPUFramework/ConditionNode.h"
#include "DPUFramework/DPUTestLogics.h"
#include "SagaStatsLog.h"
#include "Kismet/KismetSystemLibrary.h"

// ============================================================================
// 辅助：构建条件树节点
// ============================================================================

static UConditionNode_FactQuery* MakeFactQuery(UObject* Outer, FName FactKey, FName MethodName = NAME_None, bool bReverse = false)
{
	UConditionNode_FactQuery* Node = NewObject<UConditionNode_FactQuery>(Outer);
	Node->FactKey = FactKey;
	Node->MethodName = MethodName;
	Node->bReverse = bReverse;
	return Node;
}

static UConditionNode_And* MakeAnd(UObject* Outer, const TArray<UConditionNode*>& Children, bool bReverse = false)
{
	UConditionNode_And* Node = NewObject<UConditionNode_And>(Outer);
	for (UConditionNode* Child : Children)
	{
		Node->Children.Add(Child);
	}
	Node->bReverse = bReverse;
	return Node;
}

static UDPUDefinition* MakeDPU(
	UObject* Outer, FName Name, UConditionNode* Condition,
	const TArray<FName>& Produces, TSubclassOf<UDPULogicBase> Logic)
{
	UDPUDefinition* Def = NewObject<UDPUDefinition>(Outer);
	Def->DPUName = Name;
	Def->Produces = Produces;
	Def->LogicClass = Logic;
	Def->Condition = Condition;
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
	using namespace SekiroFactKeys;

	Pipeline = NewObject<UPipelineAsset>(this);
	Pipeline->PipelineName = FName("SekiroMVPv4");
	Pipeline->bAutoExportMermaid = true;

	// ================================================================
	// DPU_Mixup：无 Condition
	// Produces: MixupResult
	// ================================================================
	Pipeline->DPUDefinitions.Add(MakeDPU(this, "DPU_Mixup", nullptr,
		{MixupResult}, UDPULogic_TestMixup::StaticClass()));

	// ================================================================
	// DPU_Guard: MixupResult.IsGuard && !LightningInAirResult
	// Produces: GuardResult
	// ================================================================
	Pipeline->DPUDefinitions.Add(MakeDPU(this, "DPU_Guard",
		MakeAnd(this, {
			MakeFactQuery(this, MixupResult, FName("IsGuard")),
			MakeFactQuery(this, LightningInAirResult, NAME_None, true) // !信号
		}),
		{GuardResult}, UDPULogic_TestGuard::StaticClass()));

	// ================================================================
	// 共用条件模式：!(MixupResult.IsGuard && GuardResult.IsGuardSuccess) && !LightningInAirResult
	// 用于 Death, Collapse, Hurt, Toughness, SuperArmor
	// ================================================================
	auto MakeHurtCondition = [this]() -> UConditionNode*
	{
		return MakeAnd(this, {
			MakeAnd(this, {
				MakeFactQuery(this, MixupResult, FName("IsGuard")),
				MakeFactQuery(this, GuardResult, FName("IsGuardSuccess"))
			}, true), // !(IsGuard && IsGuardSuccess)
			MakeFactQuery(this, LightningInAirResult, NAME_None, true) // !LightningInAirResult
		});
	};

	// DPU_Death
	Pipeline->DPUDefinitions.Add(MakeDPU(this, "DPU_Death", MakeHurtCondition(),
		{DeathResult}, UDPULogic_TestDeath::StaticClass()));

	// DPU_Collapse
	Pipeline->DPUDefinitions.Add(MakeDPU(this, "DPU_Collapse", MakeHurtCondition(),
		{CollapseResult}, UDPULogic_TestCollapse::StaticClass()));

	// DPU_Hurt
	Pipeline->DPUDefinitions.Add(MakeDPU(this, "DPU_Hurt", MakeHurtCondition(),
		{HurtResult}, UDPULogic_TestHurt::StaticClass()));

	// ================================================================
	// DPU_CollapseGuard: GuardResult.IsGuardSuccess && !GuardResult.IsJustGuard && !LightningInAirResult
	// ================================================================
	Pipeline->DPUDefinitions.Add(MakeDPU(this, "DPU_CollapseGuard",
		MakeAnd(this, {
			MakeFactQuery(this, GuardResult, FName("IsGuardSuccess")),
			MakeFactQuery(this, GuardResult, FName("IsJustGuard"), true), // !IsJustGuard
			MakeFactQuery(this, LightningInAirResult, NAME_None, true)
		}),
		{CollapseResult}, UDPULogic_TestCollapseGuard::StaticClass()));

	// ================================================================
	// DPU_CollapseJustGuard: GuardResult.IsGuardSuccess && GuardResult.IsJustGuard && !LightningInAirResult
	// ================================================================
	Pipeline->DPUDefinitions.Add(MakeDPU(this, "DPU_CollapseJustGuard",
		MakeAnd(this, {
			MakeFactQuery(this, GuardResult, FName("IsGuardSuccess")),
			MakeFactQuery(this, GuardResult, FName("IsJustGuard")),
			MakeFactQuery(this, LightningInAirResult, NAME_None, true)
		}),
		{CollapseJustGuardResult}, UDPULogic_TestCollapseJustGuard::StaticClass()));

	// ================================================================
	// DPU_AttackerBound: GuardResult.IsGuardSuccess && GuardResult.IsJustGuard && !LightningInAirResult
	// ================================================================
	Pipeline->DPUDefinitions.Add(MakeDPU(this, "DPU_AttackerBound",
		MakeAnd(this, {
			MakeFactQuery(this, GuardResult, FName("IsGuardSuccess")),
			MakeFactQuery(this, GuardResult, FName("IsJustGuard")),
			MakeFactQuery(this, LightningInAirResult, NAME_None, true)
		}),
		{AttackerBoundResult}, UDPULogic_TestAttackerBound::StaticClass()));

	// ================================================================
	// DPU_Poison: !(GuardResult.IsJustGuard && IsPlayer 上下文)
	// 注：IsPlayer 是事件上下文字段（无 DPU 生产者），不参与 DAG
	// 简化为：!GuardResult.IsJustGuard（IsPlayer 条件在 CanActive 中处理，现阶段省略）
	// ================================================================
	Pipeline->DPUDefinitions.Add(MakeDPU(this, "DPU_Poison",
		MakeFactQuery(this, GuardResult, FName("IsJustGuard"), true), // !IsJustGuard
		{PoisonResult}, UDPULogic_TestPoison::StaticClass()));

	// DPU_Toughness
	Pipeline->DPUDefinitions.Add(MakeDPU(this, "DPU_Toughness", MakeHurtCondition(),
		{ToughnessResult}, UDPULogic_TestToughness::StaticClass()));

	// DPU_SuperArmor
	Pipeline->DPUDefinitions.Add(MakeDPU(this, "DPU_SuperArmor", MakeHurtCondition(),
		{SuperArmorResult}, UDPULogic_TestSuperArmor::StaticClass()));

	// ================================================================
	// DPU_LightningOnGround: DC 上下文 Lightning=true && !IsInAir
	// Lightning 和 IsInAir 是事件上下文字段，用标量兼容接口检查
	// 这里用 FactQuery 信号检查：Lightning 和 IsInAir 是 SetBool 写入的标量
	// ================================================================
	Pipeline->DPUDefinitions.Add(MakeDPU(this, "DPU_LightningOnGround",
		MakeAnd(this, {
			MakeFactQuery(this, FName("Lightning")), // DC 初始字段，信号检查
			MakeFactQuery(this, FName("IsInAir"), NAME_None, true)
		}),
		{LightningOnGroundResult}, UDPULogic_TestLightningOnGround::StaticClass()));

	// DPU_LightningInAir: Lightning && IsInAir
	Pipeline->DPUDefinitions.Add(MakeDPU(this, "DPU_LightningInAir",
		MakeAnd(this, {
			MakeFactQuery(this, FName("Lightning")),
			MakeFactQuery(this, FName("IsInAir"))
		}),
		{LightningInAirResult}, UDPULogic_TestLightningInAir::StaticClass()));

	// Build 烘焙
	FPipelineSortResult SortResult = Pipeline->Build();

	PrintToScreen(FString::Printf(TEXT("=== Sekiro MVP v4 Fact Pipeline Built: %d DPUs ==="), Pipeline->DPUDefinitions.Num()), FColor::Cyan);

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
	// guard_level=0 → Mixup 产出 bIsGuard=false

	TArray<FDPUExecutionEntry> Log = Pipeline->Execute(DC);
	PrintScenarioResult(TEXT("1. Normal Slash Hit"), DC, Log);
}

void ADPUPipelineTestActor::RunScenario_Guard()
{
	Pipeline->ScenarioLabel = TEXT("2.Guard");
	UDamageContext* DC = NewObject<UDamageContext>(this);
	DC->SetFloat(FName("DmgLevel"), 3.0f);
	DC->SetFloat(FName("guard_level"), 3.0f); // == DmgLevel → 格挡但非弹刀
	DC->SetFloat(FName("CurrentHP"), 100.0f);

	TArray<FDPUExecutionEntry> Log = Pipeline->Execute(DC);
	PrintScenarioResult(TEXT("2. Guard (non-JustGuard)"), DC, Log);
}

void ADPUPipelineTestActor::RunScenario_JustGuard()
{
	Pipeline->ScenarioLabel = TEXT("3.JustGuard");
	UDamageContext* DC = NewObject<UDamageContext>(this);
	DC->SetFloat(FName("DmgLevel"), 3.0f);
	DC->SetFloat(FName("guard_level"), 5.0f); // > DmgLevel → 弹刀
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

	// DC 结果：显示 Fact 的存在性和关键字段
	PrintToScreen(TEXT("  DC Facts:"), FColor::Yellow);
	for (const auto& Pair : DC->GetAllFacts())
	{
		if (DC->GetContextFieldNames().Contains(Pair.Key)) continue;

		FString ValueStr;
		if (!Pair.Value.IsValid())
		{
			ValueStr = TEXT("(invalid)");
		}
		else if (const UScriptStruct* Struct = Pair.Value.GetScriptStruct())
		{
			// 信号 Fact：空结构体 → 显示 "signal"
			if (Struct->GetPropertiesSize() == Struct->GetMinAlignment()) // 无自有字段
			{
				ValueStr = TEXT("(signal)");
			}
			else
			{
				// 复杂 Fact：用 DumpToString 显示内部字段
				ValueStr = FString::Printf(TEXT("[%s]"), *Struct->GetName());
			}
		}

		FColor ValColor = DC->HasFact(Pair.Key) ? FColor::Green : FColor::Red;
		PrintToScreen(FString::Printf(TEXT("    %s = %s"), *Pair.Key.ToString(), *ValueStr), ValColor);
	}
}
