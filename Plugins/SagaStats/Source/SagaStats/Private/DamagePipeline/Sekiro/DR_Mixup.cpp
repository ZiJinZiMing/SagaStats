/***************************************************************************************************************
* Plugin:       SagaStats
* Author:       Jinming Zhang
* Description:  SagaStats offers modular damage process and meter systems to support adaptable status management
****************************************************************************************************************/

// DR_Mixup.cpp — 猜拳判定机制实现
#include "DamagePipeline/Sekiro/DR_Mixup.h"
#include "DamagePipeline/Sekiro/SekiroAttackContext.h"

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
// Operation
// ============================================================================

void UDamageOperation_Mixup::Execute_Implementation(UDamageContext* Context, FInstancedStruct& OutEffect)
{
	const FSekiroAttackContext* Atk = ReadEffect<FSekiroAttackContext>(Context);
	FMixupEffect Result;
	if (Atk)
	{
		Result.bIsGuard = Atk->GuardLevel > 0.f;
		Result.bIsJustGuard = Atk->GuardLevel > Atk->DmgLevel;
	}
	OutEffect = FInstancedStruct::Make<FMixupEffect>(Result);
}
