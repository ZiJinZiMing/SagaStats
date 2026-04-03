// DPU_Guard.cpp — 防御判定机制实现
#include "DPUFramework/Sekiro/DPU_Guard.h"
#include "DPUFramework/Sekiro/DPU_Mixup.h"

// ============================================================================
// ConditionNode 领域方法
// ============================================================================

bool UConditionNode_Guard::IsGuardSuccess(const UDamageContext* DC) const
{
	const FGuardResult* F = DC->GetFact<FGuardResult>(ResolvedProducerDPU);
	return F ? F->bGuardSuccess : false;
}

bool UConditionNode_Guard::IsJustGuard(const UDamageContext* DC) const
{
	const FGuardResult* F = DC->GetFact<FGuardResult>(ResolvedProducerDPU);
	return F ? F->bIsJustGuard : false;
}

// ============================================================================
// Logic
// ============================================================================

FInstancedStruct UDPULogic_Guard::Execute_Implementation(const UDamageContext* DC)
{
	const FMixupResult* Mixup = DC->GetFact<FMixupResult>(FName("Mixup"));

	FGuardResult Result;
	Result.bGuardSuccess = Mixup ? Mixup->bIsGuard : false;
	Result.bIsJustGuard = Mixup ? Mixup->bIsJustGuard : false;
	return FInstancedStruct::Make<FGuardResult>(Result);
}
