// DPU_AttackerBound.cpp — 攻击者绑定机制实现
#include "DPUFramework/Sekiro/DPU_AttackerBound.h"

// ============================================================================
// ConditionNode 领域方法
// ============================================================================

bool UConditionNode_AttackerBound::IsBound(const UDamageContext* DC) const
{
	const FAttackerBoundResult* F = DC->GetFact<FAttackerBoundResult>(ResolvedProducerDPU);
	return F ? F->bIsBound : false;
}

// ============================================================================
// Logic
// ============================================================================

FInstancedStruct UDPULogic_AttackerBound::Execute_Implementation(const UDamageContext* DC)
{
	FAttackerBoundResult Result;
	Result.bIsBound = true;
	return FInstancedStruct::Make<FAttackerBoundResult>(Result);
}
