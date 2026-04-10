// DR_Mixup.cpp — 猜拳判定机制实现
#include "DamageFramework/Sekiro/DR_Mixup.h"
#include "DamageFramework/Sekiro/DR_AttackContext.h"

// ============================================================================
// Condition
// ============================================================================

bool UDamageCondition_IsGuard::Evaluate_Implementation(const UDamageContext* Context, const FInstancedStruct& ConsumedEffect) const
{
	const FMixupEffect* F = ConsumedEffect.GetPtr<FMixupEffect>();
	return F ? F->bIsGuard : false;
}

bool UDamageCondition_IsJustGuard::Evaluate_Implementation(const UDamageContext* Context, const FInstancedStruct& ConsumedEffect) const
{
	const FMixupEffect* F = ConsumedEffect.GetPtr<FMixupEffect>();
	return F ? F->bIsJustGuard : false;
}

// ============================================================================
// Logic
// ============================================================================

void UDamageOperation_Mixup::Execute_Implementation(UDamageContext* Context, FInstancedStruct& OutEffect)
{
	const FSekiroAttackContext* Atk = Context->GetEffect<FSekiroAttackContext>();
	FMixupEffect Result;
	if (Atk)
	{
		Result.bIsGuard = Atk->GuardLevel > 0.f;
		Result.bIsJustGuard = Atk->GuardLevel > Atk->DmgLevel;
	}
	OutEffect = FInstancedStruct::Make<FMixupEffect>(Result);
}
