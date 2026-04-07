// DPU_Collapse.cpp — 崩架势机制实现
#include "DPUFramework/Sekiro/DPU_Collapse.h"

// ============================================================================
// Condition
// ============================================================================

bool UDPUCondition_CollapseIsCollapse::Evaluate_Implementation(const UDamageContext* DC, const FInstancedStruct& ConsumedFact) const
{
	const FCollapseResult* F = ConsumedFact.GetPtr<FCollapseResult>();
	return F ? F->bIsCollapse : false;
}

bool UDPUCondition_CollapseGuardIsCollapse::Evaluate_Implementation(const UDamageContext* DC, const FInstancedStruct& ConsumedFact) const
{
	const FCollapseGuardResult* F = ConsumedFact.GetPtr<FCollapseGuardResult>();
	return F ? F->bIsCollapse : false;
}

// ============================================================================
// Logic
// ============================================================================

void UDPULogic_Collapse::Execute_Implementation(UDamageContext* DC, FInstancedStruct& OutFact)
{
	FCollapseResult Result;
	Result.bIsCollapse = true;
	OutFact = FInstancedStruct::Make<FCollapseResult>(Result);
}

void UDPULogic_CollapseGuard::Execute_Implementation(UDamageContext* DC, FInstancedStruct& OutFact)
{
	FCollapseGuardResult Result;
	Result.bIsCollapse = true;
	OutFact = FInstancedStruct::Make<FCollapseGuardResult>(Result);
}
