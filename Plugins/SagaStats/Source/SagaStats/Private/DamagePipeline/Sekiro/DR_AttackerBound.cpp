/***************************************************************************************************************
* Plugin:       SagaStats
* Author:       Jinming Zhang
* Description:  SagaStats offers modular damage process and meter systems to support adaptable status management
****************************************************************************************************************/

// DR_AttackerBound.cpp — 攻击者绑定机制实现
#include "DamagePipeline/Sekiro/DR_AttackerBound.h"

// ============================================================================
// Condition
// ============================================================================

bool UDamageCondition_IsBound::Evaluate_Implementation(const UDamageContext* Context, const FInstancedStruct& ConsumedEffect) const
{
	const FAttackerBoundEffect* F = ConsumedEffect.GetPtr<FAttackerBoundEffect>();
	return F ? F->bIsBound : false;
}

// ============================================================================
// Operation
// ============================================================================

void UDamageOperation_AttackerBound::Execute_Implementation(UDamageContext* Context, FInstancedStruct& OutEffect)
{
	FAttackerBoundEffect Result;
	Result.bIsBound = true;
	OutEffect = FInstancedStruct::Make<FAttackerBoundEffect>(Result);
}
