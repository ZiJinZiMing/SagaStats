// DR_Mixup.cpp — 猜拳判定机制实现
#include "DamageFramework/Sekiro/DR_Mixup.h"

// ============================================================================
// Condition
// ============================================================================

bool UDamageCondition_IsGuard::Evaluate_Implementation(const UDamageContext* Context, const FInstancedStruct& ConsumedFact) const
{
	const FMixupEffect* F = ConsumedFact.GetPtr<FMixupEffect>();
	return F ? F->bIsGuard : false;
}

bool UDamageCondition_IsJustGuard::Evaluate_Implementation(const UDamageContext* Context, const FInstancedStruct& ConsumedFact) const
{
	const FMixupEffect* F = ConsumedFact.GetPtr<FMixupEffect>();
	return F ? F->bIsJustGuard : false;
}

// ============================================================================
// Logic
// ============================================================================

void UDamageOperation_Mixup::Execute_Implementation(UDamageContext* Context, FInstancedStruct& OutEffect)
{
	float GuardLevel = Context->GetFloat(FName("guard_level"));
	float DmgLevel = Context->GetFloat(FName("DmgLevel"));
	FMixupEffect Result;
	Result.bIsGuard = GuardLevel > 0.f;
	Result.bIsJustGuard = GuardLevel > DmgLevel;
	OutEffect = FInstancedStruct::Make<FMixupEffect>(Result);
}
