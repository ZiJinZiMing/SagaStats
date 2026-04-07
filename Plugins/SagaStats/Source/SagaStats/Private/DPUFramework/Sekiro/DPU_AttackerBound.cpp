// DPU_AttackerBound.cpp — 攻击者绑定机制实现
#include "DPUFramework/Sekiro/DPU_AttackerBound.h"

// ============================================================================
// Condition
// ============================================================================

bool UDPUCondition_IsBound::Evaluate_Implementation(const UDamageContext* DC, const FInstancedStruct& ConsumedFact) const
{
	const FAttackerBoundResult* F = ConsumedFact.GetPtr<FAttackerBoundResult>();
	return F ? F->bIsBound : false;
}

// ============================================================================
// Logic
// ============================================================================

void UDPULogic_AttackerBound::Execute_Implementation(UDamageContext* DC, FInstancedStruct& OutFact)
{
	FAttackerBoundResult Result;
	Result.bIsBound = true;
	OutFact = FInstancedStruct::Make<FAttackerBoundResult>(Result);
}
