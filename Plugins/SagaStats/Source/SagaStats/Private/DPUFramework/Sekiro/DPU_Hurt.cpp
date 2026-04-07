// DPU_Hurt.cpp — 受击判定机制实现
#include "DPUFramework/Sekiro/DPU_Hurt.h"

// ============================================================================
// Condition
// ============================================================================

bool UDPUCondition_IsHurt::Evaluate_Implementation(const UDamageContext* DC, const FInstancedStruct& ConsumedFact) const
{
	const FHurtResult* F = ConsumedFact.GetPtr<FHurtResult>();
	return F ? F->bIsHurt : false;
}

// ============================================================================
// Logic
// ============================================================================

void UDPULogic_Hurt::Execute_Implementation(UDamageContext* DC, FInstancedStruct& OutFact)
{
	FHurtResult Result;
	Result.bIsHurt = true;
	OutFact = FInstancedStruct::Make<FHurtResult>(Result);
}
