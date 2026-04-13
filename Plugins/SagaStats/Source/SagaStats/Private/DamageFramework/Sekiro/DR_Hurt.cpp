// DR_Hurt.cpp — 受击判定机制实现
#include "DamageFramework/Sekiro/DR_Hurt.h"

// ============================================================================
// Condition
// ============================================================================

bool UDamageCondition_IsHurt::Evaluate_Implementation(const UDamageContext* Context, const FInstancedStruct& ConsumedEffect) const
{
	const FHurtEffect* F = ConsumedEffect.GetPtr<FHurtEffect>();
	return F ? F->bIsHurt : false;
}

// ============================================================================
// Operation
// ============================================================================

void UDamageOperation_Hurt::Execute_Implementation(UDamageContext* Context, FInstancedStruct& OutEffect)
{
	FHurtEffect Result;
	Result.bIsHurt = true;
	OutEffect = FInstancedStruct::Make<FHurtEffect>(Result);
}
