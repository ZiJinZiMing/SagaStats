// SekiroFacts.cpp — 只狼 MVP Fact 领域方法注册
#include "DPUFramework/SekiroFacts.h"
#include "DPUFramework/FactMethodRegistry.h"
#include "SagaStatsLog.h"

void RegisterSekiroFactMethods(UFactMethodRegistry* Registry)
{
	if (!Registry)
	{
		UE_LOG(LogSagaStats, Error, TEXT("RegisterSekiroFactMethods: Registry 为空"));
		return;
	}

	// ================================================================
	// FMixupResult 领域方法
	// ================================================================
	Registry->Register<FMixupResult>(FName("IsGuard"),
		[](const FMixupResult& R) { return R.bIsGuard; });

	Registry->Register<FMixupResult>(FName("IsJustGuard"),
		[](const FMixupResult& R) { return R.bIsJustGuard; });

	Registry->Register<FMixupResult>(FName("IsGuardSuccess"),
		[](const FMixupResult& R) { return R.bIsGuard; }); // 格挡成功 = 格挡触发

	// ================================================================
	// FGuardResult 领域方法
	// ================================================================
	Registry->Register<FGuardResult>(FName("IsGuardSuccess"),
		[](const FGuardResult& R) { return R.bGuardSuccess; });

	Registry->Register<FGuardResult>(FName("IsJustGuard"),
		[](const FGuardResult& R) { return R.bIsJustGuard; });

	// ================================================================
	// FHurtResult 领域方法
	// ================================================================
	Registry->Register<FHurtResult>(FName("IsHurt"),
		[](const FHurtResult& R) { return R.bIsHurt; });

	// ================================================================
	// FDeathResult 领域方法
	// ================================================================
	Registry->Register<FDeathResult>(FName("IsDead"),
		[](const FDeathResult& R) { return R.bIsDead; });

	// ================================================================
	// FCollapseResult 领域方法
	// ================================================================
	Registry->Register<FCollapseResult>(FName("IsCollapse"),
		[](const FCollapseResult& R) { return R.bIsCollapse; });

	// ================================================================
	// FPoisonResult 领域方法
	// ================================================================
	Registry->Register<FPoisonResult>(FName("IsPoisoned"),
		[](const FPoisonResult& R) { return R.bIsPoisoned; });

	// ================================================================
	// FAttackerBoundResult 领域方法
	// ================================================================
	Registry->Register<FAttackerBoundResult>(FName("IsBound"),
		[](const FAttackerBoundResult& R) { return R.bIsBound; });

	// ================================================================
	// Fact Key → Type 映射注册（编辑器联动下拉用）
	// ================================================================

	// 复杂 Fact
	Registry->RegisterFactType<FMixupResult>(FName("MixupResult"));
	Registry->RegisterFactType<FGuardResult>(FName("GuardResult"));
	Registry->RegisterFactType<FHurtResult>(FName("HurtResult"));
	Registry->RegisterFactType<FDeathResult>(FName("DeathResult"));
	Registry->RegisterFactType<FCollapseResult>(FName("CollapseResult"));
	Registry->RegisterFactType<FPoisonResult>(FName("PoisonResult"));
	Registry->RegisterFactType<FAttackerBoundResult>(FName("AttackerBoundResult"));

	// 信号 Fact（无领域方法，但需要注册 Key → Type 映射）
	Registry->RegisterFactType<FLightningInAirSignal>(FName("LightningInAirResult"));
	Registry->RegisterFactType<FLightningOnGroundSignal>(FName("LightningOnGroundResult"));
	Registry->RegisterFactType<FToughnessSignal>(FName("ToughnessResult"));
	Registry->RegisterFactType<FSuperArmorSignal>(FName("SuperArmorResult"));
	Registry->RegisterFactType<FCollapseJustGuardSignal>(FName("CollapseJustGuardResult"));

	UE_LOG(LogSagaStats, Log, TEXT("只狼 MVP Fact 领域方法 + Key→Type 映射注册完成"));
}
