// DPU_Mixup.cpp — 猜拳判定机制实现
#include "DPUFramework/Sekiro/DPU_Mixup.h"

// ============================================================================
// ConditionNode 领域方法
// ============================================================================

bool UConditionNode_Mixup::IsGuard(const UDamageContext* DC) const
{
	const FMixupResult* F = DC->GetFact<FMixupResult>(ResolvedProducerDPU);
	return F ? F->bIsGuard : false;
}

bool UConditionNode_Mixup::IsJustGuard(const UDamageContext* DC) const
{
	const FMixupResult* F = DC->GetFact<FMixupResult>(ResolvedProducerDPU);
	return F ? F->bIsJustGuard : false;
}

bool UConditionNode_Mixup::IsGuardSuccess(const UDamageContext* DC) const
{
	const FMixupResult* F = DC->GetFact<FMixupResult>(ResolvedProducerDPU);
	return F ? F->bIsGuard : false;
}

// ============================================================================
// Logic
// ============================================================================

FInstancedStruct UDPULogic_Mixup::Execute_Implementation(const UDamageContext* DC)
{
	float GuardLevel = DC->GetFloat(FName("guard_level"));
	float DmgLevel = DC->GetFloat(FName("DmgLevel"));
	FMixupResult Result;
	Result.bIsGuard = GuardLevel > 0.f;
	Result.bIsJustGuard = GuardLevel > DmgLevel;
	return FInstancedStruct::Make<FMixupResult>(Result);
}
