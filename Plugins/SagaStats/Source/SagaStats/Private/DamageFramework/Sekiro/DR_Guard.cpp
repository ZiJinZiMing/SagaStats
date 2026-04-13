// DR_Guard.cpp — 防御判定机制实现
#include "DamageFramework/Sekiro/DR_Guard.h"
#include "DamageFramework/Sekiro/DR_Mixup.h"

// ============================================================================
// Condition
// ============================================================================

bool UDamageCondition_GuardSuccess::Evaluate_Implementation(const UDamageContext* Context, const FInstancedStruct& ConsumedEffect) const
{
	const FGuardEffect* F = ConsumedEffect.GetPtr<FGuardEffect>();
	return F ? F->bGuardSuccess : false;
}

bool UDamageCondition_GuardIsJustGuard::Evaluate_Implementation(const UDamageContext* Context, const FInstancedStruct& ConsumedEffect) const
{
	const FGuardEffect* F = ConsumedEffect.GetPtr<FGuardEffect>();
	return F ? F->bIsJustGuard : false;
}

// ============================================================================
// Operation
// ============================================================================

void UDamageOperation_Guard::Execute_Implementation(UDamageContext* Context, FInstancedStruct& OutEffect)
{
	const FMixupEffect* Mixup = Context->GetEffect<FMixupEffect>();

	FGuardEffect Result;
	Result.bGuardSuccess = Mixup ? Mixup->bIsGuard : false;
	Result.bIsJustGuard = Mixup ? Mixup->bIsJustGuard : false;
	OutEffect = FInstancedStruct::Make<FGuardEffect>(Result);
}
