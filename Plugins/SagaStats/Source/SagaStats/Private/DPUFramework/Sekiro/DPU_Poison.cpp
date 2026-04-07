// DPU_Poison.cpp — 中毒判定机制实现
#include "DPUFramework/Sekiro/DPU_Poison.h"

// ============================================================================
// Condition
// ============================================================================

bool UDPUCondition_IsPoisoned::Evaluate_Implementation(const UDamageContext* DC, const FInstancedStruct& ConsumedFact) const
{
	const FPoisonResult* F = ConsumedFact.GetPtr<FPoisonResult>();
	return F ? F->bIsPoisoned : false;
}

// ============================================================================
// Logic
// ============================================================================

void UDPULogic_Poison::Execute_Implementation(UDamageContext* DC, FInstancedStruct& OutFact)
{
	FPoisonResult Result;
	Result.bIsPoisoned = true;
	OutFact = FInstancedStruct::Make<FPoisonResult>(Result);
}
