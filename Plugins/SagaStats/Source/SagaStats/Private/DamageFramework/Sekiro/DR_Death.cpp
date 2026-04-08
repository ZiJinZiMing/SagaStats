// DR_Death.cpp — 死亡判定机制实现
#include "DamageFramework/Sekiro/DR_Death.h"

// ============================================================================
// Condition
// ============================================================================

bool UDamageCondition_IsDead::Evaluate_Implementation(const UDamageContext* Context, const FInstancedStruct& ConsumedFact) const
{
	const FDeathEffect* F = ConsumedFact.GetPtr<FDeathEffect>();
	return F ? F->bIsDead : false;
}

// ============================================================================
// Logic
// ============================================================================

void UDamageOperation_Death::Execute_Implementation(UDamageContext* Context, FInstancedStruct& OutEffect)
{
	float CurrentHP = Context->GetFloat(FName("CurrentHP"));
	float DmgLevel = Context->GetFloat(FName("DmgLevel"));

	FDeathEffect Result;
	Result.bIsDead = (CurrentHP - DmgLevel) <= 0.f;
	OutEffect = FInstancedStruct::Make<FDeathEffect>(Result);
}
