// DPU_Collapse.cpp — 崩架势机制实现
#include "DPUFramework/Sekiro/DPU_Collapse.h"

// ============================================================================
// ConditionNode 领域方法
// ============================================================================

bool UConditionNode_Collapse::IsCollapse(const UDamageContext* DC) const
{
	const FCollapseResult* F = DC->GetFact<FCollapseResult>(ResolvedProducerDPU);
	return F ? F->bIsCollapse : false;
}

bool UConditionNode_CollapseGuard::IsCollapse(const UDamageContext* DC) const
{
	const FCollapseGuardResult* F = DC->GetFact<FCollapseGuardResult>(ResolvedProducerDPU);
	return F ? F->bIsCollapse : false;
}

// ============================================================================
// Logic
// ============================================================================

FInstancedStruct UDPULogic_Collapse::Execute_Implementation(const UDamageContext* DC)
{
	FCollapseResult Result;
	Result.bIsCollapse = true;
	return FInstancedStruct::Make<FCollapseResult>(Result);
}

FInstancedStruct UDPULogic_CollapseGuard::Execute_Implementation(const UDamageContext* DC)
{
	FCollapseGuardResult Result;
	Result.bIsCollapse = true;
	return FInstancedStruct::Make<FCollapseGuardResult>(Result);
}
