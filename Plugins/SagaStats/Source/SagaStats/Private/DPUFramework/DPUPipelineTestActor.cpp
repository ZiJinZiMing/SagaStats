// DPUPipelineTestActor.cpp — DPU Pipeline 可视化运行时测试 Actor
#include "DPUFramework/DPUPipelineTestActor.h"
#include "DPUFramework/DamageContext.h"
#include "DPUFramework/StringConditionExpression.h"
#include "DPUFramework/DPUTestLogics.h"
#include "SagaStatsLog.h"
#include "Engine/Engine.h"
#include "Kismet/KismetSystemLibrary.h"

// ============================================================================
// 辅助方法：创建 DPU 定义
// ============================================================================
static UDPUDefinition* MakeRTDPU(
	UObject* Outer, FName Name, const FString& CondExpr,
	const TArray<FName>& Produces, TSubclassOf<UDPULogicBase> Logic)
{
	UDPUDefinition* Def = NewObject<UDPUDefinition>(Outer);
	Def->DPUName = Name;
	Def->Produces = Produces;
	Def->LogicClass = Logic;
	if (!CondExpr.IsEmpty())
	{
		Def->Condition = UStringConditionExpression::Create(Def, CondExpr);
	}
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
	PipelineManager = NewObject<UPipelineManager>(this);

	SekiroDPUs.Empty();
	SekiroDPUs.Add(MakeRTDPU(this, "DPU_Mixup", "",
		{"Guard"}, UDPULogic_TestMixup::StaticClass()));
	SekiroDPUs.Add(MakeRTDPU(this, "DPU_Guard", "Guard && !IsLightningInAir",
		{"IsJustGuard", "GuardSuccess"}, UDPULogic_TestGuard::StaticClass()));
	SekiroDPUs.Add(MakeRTDPU(this, "DPU_Death", "!(Guard && GuardSuccess) && !IsLightningInAir",
		{"IsDeath"}, UDPULogic_TestDeath::StaticClass()));
	SekiroDPUs.Add(MakeRTDPU(this, "DPU_Collapse", "!(Guard && GuardSuccess) && !IsLightningInAir",
		{"IsCollapse"}, UDPULogic_TestCollapse::StaticClass()));
	SekiroDPUs.Add(MakeRTDPU(this, "DPU_Hurt", "!(Guard && GuardSuccess) && !IsLightningInAir",
		{"HurtExecuted"}, UDPULogic_TestHurt::StaticClass()));
	SekiroDPUs.Add(MakeRTDPU(this, "DPU_CollapseGuard", "GuardSuccess && !IsJustGuard && !IsLightningInAir",
		{"IsCollapse"}, UDPULogic_TestCollapseGuard::StaticClass()));
	SekiroDPUs.Add(MakeRTDPU(this, "DPU_CollapseJustGuard", "GuardSuccess && IsJustGuard && !IsLightningInAir",
		{}, UDPULogic_TestCollapseJustGuard::StaticClass()));
	SekiroDPUs.Add(MakeRTDPU(this, "DPU_AttackerBound", "GuardSuccess && IsJustGuard && !IsLightningInAir",
		{"IsBound"}, UDPULogic_TestAttackerBound::StaticClass()));
	SekiroDPUs.Add(MakeRTDPU(this, "DPU_Poison", "!(IsJustGuard && IsPlayer)",
		{"IsPoisoned"}, UDPULogic_TestPoison::StaticClass()));
	SekiroDPUs.Add(MakeRTDPU(this, "DPU_Toughness", "!(Guard && GuardSuccess) && !IsLightningInAir",
		{"IsTough"}, UDPULogic_TestToughness::StaticClass()));
	SekiroDPUs.Add(MakeRTDPU(this, "DPU_SuperArmor", "!(Guard && GuardSuccess) && !IsLightningInAir",
		{"IsInSuperArmor"}, UDPULogic_TestSuperArmor::StaticClass()));
	SekiroDPUs.Add(MakeRTDPU(this, "DPU_LightningOnGround", "Lightning && !IsInAir",
		{}, UDPULogic_TestLightningOnGround::StaticClass()));
	SekiroDPUs.Add(MakeRTDPU(this, "DPU_LightningInAir", "Lightning && IsInAir",
		{"IsLightningInAir"}, UDPULogic_TestLightningInAir::StaticClass()));

	PrintToScreen(FString::Printf(TEXT("=== Sekiro MVP v4 Pipeline Built: %d DPUs ==="), SekiroDPUs.Num()), FColor::Cyan);

	// 打印拓扑排序结果
	TArray<UDPUDefinition*> RawPtrs;
	for (auto& D : SekiroDPUs) RawPtrs.Add(D.Get());
	FPipelineSortResult SortResult = UPipelineManager::StableTopologicalSort(RawPtrs);

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
	PrintToScreen(TEXT(""), FColor::White);
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
// 场景 1：普通斩击命中 — 无格挡，无雷电
// ============================================================================
void ADPUPipelineTestActor::RunScenario_NormalHit()
{
	UDamageContext* DC = NewObject<UDamageContext>(this);
	DC->SetFloat(FName("DmgLevel"), 3.0f);
	DC->SetFloat(FName("CurrentHP"), 100.0f);
	// guard_level=0 -> Mixup 设置 Guard=false

	TArray<UDPUDefinition*> RawPtrs;
	for (auto& D : SekiroDPUs) RawPtrs.Add(D.Get());

	TArray<FDPUExecutionEntry> Log = PipelineManager->ExecutePipeline(RawPtrs, DC);
	PrintScenarioResult(TEXT("1. Normal Slash Hit"), DC, Log);
}

// ============================================================================
// 场景 2：格挡（非弹刀）
// ============================================================================
void ADPUPipelineTestActor::RunScenario_Guard()
{
	UDamageContext* DC = NewObject<UDamageContext>(this);
	DC->SetFloat(FName("DmgLevel"), 3.0f);
	DC->SetFloat(FName("guard_level"), 3.0f); // == DmgLevel -> 格挡成功但非弹刀
	DC->SetFloat(FName("CurrentHP"), 100.0f);

	TArray<UDPUDefinition*> RawPtrs;
	for (auto& D : SekiroDPUs) RawPtrs.Add(D.Get());

	TArray<FDPUExecutionEntry> Log = PipelineManager->ExecutePipeline(RawPtrs, DC);
	PrintScenarioResult(TEXT("2. Guard (non-JustGuard)"), DC, Log);
}

// ============================================================================
// 场景 3：弹刀 (JustGuard)
// ============================================================================
void ADPUPipelineTestActor::RunScenario_JustGuard()
{
	UDamageContext* DC = NewObject<UDamageContext>(this);
	DC->SetFloat(FName("DmgLevel"), 3.0f);
	DC->SetFloat(FName("guard_level"), 5.0f); // > DmgLevel -> 触发弹刀
	DC->SetFloat(FName("CurrentHP"), 100.0f);
	DC->SetBool(FName("IsPlayer"), true);

	TArray<UDPUDefinition*> RawPtrs;
	for (auto& D : SekiroDPUs) RawPtrs.Add(D.Get());

	TArray<FDPUExecutionEntry> Log = PipelineManager->ExecutePipeline(RawPtrs, DC);
	PrintScenarioResult(TEXT("3. JustGuard (Deflect)"), DC, Log);
}

// ============================================================================
// 场景 4：地面雷电
// ============================================================================
void ADPUPipelineTestActor::RunScenario_LightningGround()
{
	UDamageContext* DC = NewObject<UDamageContext>(this);
	DC->SetBool(FName("Lightning"), true);
	DC->SetBool(FName("IsInAir"), false);
	DC->SetFloat(FName("DmgLevel"), 3.0f);
	DC->SetFloat(FName("CurrentHP"), 100.0f);

	TArray<UDPUDefinition*> RawPtrs;
	for (auto& D : SekiroDPUs) RawPtrs.Add(D.Get());

	TArray<FDPUExecutionEntry> Log = PipelineManager->ExecutePipeline(RawPtrs, DC);
	PrintScenarioResult(TEXT("4. Lightning on Ground"), DC, Log);
}

// ============================================================================
// 场景 5：空中接雷
// ============================================================================
void ADPUPipelineTestActor::RunScenario_LightningInAir()
{
	UDamageContext* DC = NewObject<UDamageContext>(this);
	DC->SetBool(FName("Lightning"), true);
	DC->SetBool(FName("IsInAir"), true);
	DC->SetFloat(FName("DmgLevel"), 3.0f);
	DC->SetFloat(FName("CurrentHP"), 100.0f);

	TArray<UDPUDefinition*> RawPtrs;
	for (auto& D : SekiroDPUs) RawPtrs.Add(D.Get());

	TArray<FDPUExecutionEntry> Log = PipelineManager->ExecutePipeline(RawPtrs, DC);
	PrintScenarioResult(TEXT("5. Lightning in Air"), DC, Log);
}

// ============================================================================
// 屏幕输出辅助方法
// ============================================================================

void ADPUPipelineTestActor::PrintToScreen(const FString& Message, FColor Color) const
{
	// 同时输出到：屏幕（Viewport）+ Output Log 控制台
	// PrintString 参数: bPrintToScreen=true, bPrintToLog=true
	UKismetSystemLibrary::PrintString(this, Message, true, true, Color, ScreenMessageDuration);
}

void ADPUPipelineTestActor::PrintScenarioResult(
	const FString& ScenarioName, UDamageContext* DC,
	const TArray<FDPUExecutionEntry>& Log) const
{
	PrintToScreen(TEXT(""), FColor::White);
	PrintToScreen(FString::Printf(TEXT("--- %s ---"), *ScenarioName), FColor::Green);

	// 执行状态
	for (const FDPUExecutionEntry& Entry : Log)
	{
		FString Status = Entry.bExecuted ? TEXT("[EXEC]") : TEXT("[SKIP]");
		FColor Color = Entry.bExecuted ? FColor::White : FColor::Silver;
		PrintToScreen(FString::Printf(TEXT("  %s %s"), *Status, *Entry.DPUName.ToString()), Color);
	}

	// 关键 DC 结果
	PrintToScreen(TEXT("  DC Results:"), FColor::Yellow);
	for (const auto& Pair : DC->GetAllFields())
	{
		// 仅显示 DPU 产出的字段（跳过 float 类型的初始上下文）
		const FDCValue& Val = Pair.Value;
		if (Val.Type == EDCValueType::Bool)
		{
			FColor ValColor = Val.BoolValue ? FColor::Green : FColor::Red;
			PrintToScreen(FString::Printf(TEXT("    %s = %s"),
				*Pair.Key.ToString(), *Val.ToString()), ValColor);
		}
	}
}
