// DPU_Mixup.cpp — 猜拳判定机制实现
#include "DPUFramework/Sekiro/DPU_Mixup.h"

// ============================================================================
// Condition
// ============================================================================

bool UDPUCondition_IsGuard::Evaluate_Implementation(const UDamageContext* DC, const FInstancedStruct& ConsumedFact) const
{
	const FMixupResult* F = ConsumedFact.GetPtr<FMixupResult>();
	return F ? F->bIsGuard : false;
}

bool UDPUCondition_IsJustGuard::Evaluate_Implementation(const UDamageContext* DC, const FInstancedStruct& ConsumedFact) const
{
	const FMixupResult* F = ConsumedFact.GetPtr<FMixupResult>();
	return F ? F->bIsJustGuard : false;
}

// ============================================================================
// Logic
// ============================================================================

void UDPULogic_Mixup::Execute_Implementation(UDamageContext* DC, FInstancedStruct& OutFact)
{
	float GuardLevel = DC->GetFloat(FName("guard_level"));
	float DmgLevel = DC->GetFloat(FName("DmgLevel"));
	FMixupResult Result;
	Result.bIsGuard = GuardLevel > 0.f;
	Result.bIsJustGuard = GuardLevel > DmgLevel;
	OutFact = FInstancedStruct::Make<FMixupResult>(Result);
}
