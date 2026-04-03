// DPU_Hurt.cpp — 受击判定机制实现
#include "DPUFramework/Sekiro/DPU_Hurt.h"

// ============================================================================
// ConditionNode 领域方法
// ============================================================================

bool UConditionNode_Hurt::IsHurt(const UDamageContext* DC) const
{
	const FHurtResult* F = DC->GetFact<FHurtResult>(ResolvedProducerDPU);
	return F ? F->bIsHurt : false;
}

// ============================================================================
// Logic
// ============================================================================

FInstancedStruct UDPULogic_Hurt::Execute_Implementation(const UDamageContext* DC)
{
	FHurtResult Result;
	Result.bIsHurt = true;
	return FInstancedStruct::Make<FHurtResult>(Result);
}
