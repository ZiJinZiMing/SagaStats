// DPU_Death.cpp — 死亡判定机制实现
#include "DPUFramework/Sekiro/DPU_Death.h"

// ============================================================================
// ConditionNode 领域方法
// ============================================================================

bool UConditionNode_Death::IsDead(const UDamageContext* DC) const
{
	const FDeathResult* F = DC->GetFact<FDeathResult>(ResolvedProducerDPU);
	return F ? F->bIsDead : false;
}

// ============================================================================
// Logic
// ============================================================================

FInstancedStruct UDPULogic_Death::Execute_Implementation(const UDamageContext* DC)
{
	float CurrentHP = DC->GetFloat(FName("CurrentHP"));
	float DmgLevel = DC->GetFloat(FName("DmgLevel"));

	FDeathResult Result;
	Result.bIsDead = (CurrentHP - DmgLevel) <= 0.f;
	return FInstancedStruct::Make<FDeathResult>(Result);
}
