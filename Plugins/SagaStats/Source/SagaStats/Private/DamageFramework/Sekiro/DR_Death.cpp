// DR_Death.cpp — 死亡判定机制实现
#include "DamageFramework/Sekiro/DR_Death.h"
#include "DamageFramework/Sekiro/DR_AttackContext.h"

// ============================================================================
// Condition
// ============================================================================

bool UDamageCondition_IsDead::Evaluate_Implementation(const UDamageContext* Context, const FInstancedStruct& ConsumedEffect) const
{
	const FDeathEffect* F = ConsumedEffect.GetPtr<FDeathEffect>();
	return F ? F->bIsDead : false;
}

// ============================================================================
// Operation
// ============================================================================

void UDamageOperation_Death::Execute_Implementation(UDamageContext* Context, FInstancedStruct& OutEffect)
{
	const FSekiroAttackContext* Atk = Context->GetEffect<FSekiroAttackContext>();
	FDeathEffect Result;
	if (Atk)
	{
		Result.bIsDead = (Atk->CurrentHP - Atk->DmgLevel) <= 0.f;
	}
	OutEffect = FInstancedStruct::Make<FDeathEffect>(Result);
}
