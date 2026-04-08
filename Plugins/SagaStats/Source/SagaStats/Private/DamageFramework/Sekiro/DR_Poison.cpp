// DR_Poison.cpp — 中毒判定机制实现
#include "DamageFramework/Sekiro/DR_Poison.h"

// ============================================================================
// Condition
// ============================================================================

bool UDamageCondition_IsPoisoned::Evaluate_Implementation(const UDamageContext* Context, const FInstancedStruct& ConsumedFact) const
{
	const FPoisonEffect* F = ConsumedFact.GetPtr<FPoisonEffect>();
	return F ? F->bIsPoisoned : false;
}

// ============================================================================
// Logic
// ============================================================================

void UDamageOperation_Poison::Execute_Implementation(UDamageContext* Context, FInstancedStruct& OutEffect)
{
	FPoisonEffect Result;
	Result.bIsPoisoned = true;
	OutEffect = FInstancedStruct::Make<FPoisonEffect>(Result);
}
