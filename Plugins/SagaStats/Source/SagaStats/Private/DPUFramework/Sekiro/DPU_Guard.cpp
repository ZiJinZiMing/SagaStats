// DPU_Guard.cpp — 防御判定机制实现
#include "DPUFramework/Sekiro/DPU_Guard.h"
#include "DPUFramework/Sekiro/DPU_Mixup.h"

// ============================================================================
// Condition
// ============================================================================

bool UDPUCondition_GuardSuccess::Evaluate_Implementation(const UDamageContext* DC, const FInstancedStruct& ConsumedFact) const
{
	const FGuardResult* F = ConsumedFact.GetPtr<FGuardResult>();
	return F ? F->bGuardSuccess : false;
}

bool UDPUCondition_GuardIsJustGuard::Evaluate_Implementation(const UDamageContext* DC, const FInstancedStruct& ConsumedFact) const
{
	const FGuardResult* F = ConsumedFact.GetPtr<FGuardResult>();
	return F ? F->bIsJustGuard : false;
}

// ============================================================================
// Logic
// ============================================================================

void UDPULogic_Guard::Execute_Implementation(UDamageContext* DC, FInstancedStruct& OutFact)
{
	const FMixupResult* Mixup = DC->GetFact<FMixupResult>();

	FGuardResult Result;
	Result.bGuardSuccess = Mixup ? Mixup->bIsGuard : false;
	Result.bIsJustGuard = Mixup ? Mixup->bIsJustGuard : false;
	OutFact = FInstancedStruct::Make<FGuardResult>(Result);
}
