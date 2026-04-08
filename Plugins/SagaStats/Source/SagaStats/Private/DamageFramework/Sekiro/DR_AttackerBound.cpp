// DR_AttackerBound.cpp — 攻击者绑定机制实现
#include "DamageFramework/Sekiro/DR_AttackerBound.h"

// ============================================================================
// Condition
// ============================================================================

bool UDamageCondition_IsBound::Evaluate_Implementation(const UDamageContext* Context, const FInstancedStruct& ConsumedFact) const
{
	const FAttackerBoundEffect* F = ConsumedFact.GetPtr<FAttackerBoundEffect>();
	return F ? F->bIsBound : false;
}

// ============================================================================
// Logic
// ============================================================================

void UDamageOperation_AttackerBound::Execute_Implementation(UDamageContext* Context, FInstancedStruct& OutEffect)
{
	FAttackerBoundEffect Result;
	Result.bIsBound = true;
	OutEffect = FInstancedStruct::Make<FAttackerBoundEffect>(Result);
}
