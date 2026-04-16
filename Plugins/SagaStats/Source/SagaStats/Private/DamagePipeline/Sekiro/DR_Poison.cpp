/***************************************************************************************************************
* Plugin:       SagaStats
* Author:       Jinming Zhang
* Description:  SagaStats offers modular damage process and meter systems to support adaptable status management
****************************************************************************************************************/

// DR_Poison.cpp — 中毒判定机制实现
#include "DamagePipeline/Sekiro/DR_Poison.h"

// ============================================================================
// Condition
// ============================================================================

bool UDamageCondition_IsPoisoned::Evaluate_Implementation(const UDamageContext* Context, const FInstancedStruct& ConsumedEffect) const
{
	const FPoisonEffect* F = ConsumedEffect.GetPtr<FPoisonEffect>();
	return F ? F->bIsPoisoned : false;
}

// ============================================================================
// Operation
// ============================================================================

void UDamageOperation_Poison::Execute_Implementation(UDamageContext* Context, FInstancedStruct& OutEffect)
{
	FPoisonEffect Result;
	Result.bIsPoisoned = true;
	OutEffect = FInstancedStruct::Make<FPoisonEffect>(Result);
}
