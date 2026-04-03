// DPU_Poison.cpp — 中毒判定机制实现
#include "DPUFramework/Sekiro/DPU_Poison.h"

// ============================================================================
// ConditionNode 领域方法
// ============================================================================

bool UConditionNode_Poison::IsPoisoned(const UDamageContext* DC) const
{
	const FPoisonResult* F = DC->GetFact<FPoisonResult>(ResolvedProducerDPU);
	return F ? F->bIsPoisoned : false;
}

// ============================================================================
// Logic
// ============================================================================

FInstancedStruct UDPULogic_Poison::Execute_Implementation(const UDamageContext* DC)
{
	FPoisonResult Result;
	Result.bIsPoisoned = true;
	return FInstancedStruct::Make<FPoisonResult>(Result);
}
