/***************************************************************************************************************
* Plugin:       SagaStats
* Author:       Jinming Zhang
* Description:  SagaStats offers modular damage process and meter systems to support adaptable status management
****************************************************************************************************************/

// DR_Collapse.cpp — 崩架势机制实现
#include "DamagePipeline/Sekiro/DR_Collapse.h"

// ============================================================================
// Condition
// ============================================================================

bool UDamageCondition_CollapseIsCollapse::Evaluate_Implementation(const UDamageContext* Context, const FInstancedStruct& ConsumedEffect) const
{
	const FCollapseEffect* F = ConsumedEffect.GetPtr<FCollapseEffect>();
	return F ? F->bIsCollapse : false;
}

bool UDamageCondition_CollapseGuardIsCollapse::Evaluate_Implementation(const UDamageContext* Context, const FInstancedStruct& ConsumedEffect) const
{
	const FCollapseGuardEffect* F = ConsumedEffect.GetPtr<FCollapseGuardEffect>();
	return F ? F->bIsCollapse : false;
}

// ============================================================================
// Operation
// ============================================================================

void UDamageOperation_Collapse::Execute_Implementation(UDamageContext* Context, FInstancedStruct& OutEffect)
{
	FCollapseEffect Result;
	Result.bIsCollapse = true;
	OutEffect = FInstancedStruct::Make<FCollapseEffect>(Result);
}

void UDamageOperation_CollapseGuard::Execute_Implementation(UDamageContext* Context, FInstancedStruct& OutEffect)
{
	FCollapseGuardEffect Result;
	Result.bIsCollapse = true;
	OutEffect = FInstancedStruct::Make<FCollapseGuardEffect>(Result);
}
