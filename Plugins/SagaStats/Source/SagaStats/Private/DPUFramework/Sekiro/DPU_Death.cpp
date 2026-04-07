// DPU_Death.cpp — 死亡判定机制实现
#include "DPUFramework/Sekiro/DPU_Death.h"

// ============================================================================
// Condition
// ============================================================================

bool UDPUCondition_IsDead::Evaluate_Implementation(const UDamageContext* DC, const FInstancedStruct& ConsumedFact) const
{
	const FDeathResult* F = ConsumedFact.GetPtr<FDeathResult>();
	return F ? F->bIsDead : false;
}

// ============================================================================
// Logic
// ============================================================================

void UDPULogic_Death::Execute_Implementation(UDamageContext* DC, FInstancedStruct& OutFact)
{
	float CurrentHP = DC->GetFloat(FName("CurrentHP"));
	float DmgLevel = DC->GetFloat(FName("DmgLevel"));

	FDeathResult Result;
	Result.bIsDead = (CurrentHP - DmgLevel) <= 0.f;
	OutFact = FInstancedStruct::Make<FDeathResult>(Result);
}
