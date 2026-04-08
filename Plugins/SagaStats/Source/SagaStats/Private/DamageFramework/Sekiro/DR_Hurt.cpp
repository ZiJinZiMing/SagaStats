// DR_Hurt.cpp — 受击判定机制实现
#include "DamageFramework/Sekiro/DR_Hurt.h"

// ============================================================================
// Condition
// ============================================================================

bool UDamageCondition_IsHurt::Evaluate_Implementation(const UDamageContext* Context, const FInstancedStruct& ConsumedFact) const
{
	const FHurtEffect* F = ConsumedFact.GetPtr<FHurtEffect>();
	return F ? F->bIsHurt : false;
}

// ============================================================================
// Logic
// ============================================================================

void UDamageOperation_Hurt::Execute_Implementation(UDamageContext* Context, FInstancedStruct& OutEffect)
{
	FHurtEffect Result;
	Result.bIsHurt = true;
	OutEffect = FInstancedStruct::Make<FHurtEffect>(Result);
}
