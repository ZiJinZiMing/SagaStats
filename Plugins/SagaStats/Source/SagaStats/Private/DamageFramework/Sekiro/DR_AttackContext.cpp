// DR_AttackContext.cpp — 只狼攻击上下文 Condition 实现
#include "DamageFramework/Sekiro/DR_AttackContext.h"

bool UDamageCondition_IsLightning::Evaluate_Implementation(const UDamageContext* Context, const FInstancedStruct& ConsumedEffect) const
{
	if (const FSekiroAttackContext* Ctx = ConsumedEffect.GetPtr<FSekiroAttackContext>())
	{
		return Ctx->bLightning;
	}
	return false;
}

bool UDamageCondition_IsInAir::Evaluate_Implementation(const UDamageContext* Context, const FInstancedStruct& ConsumedEffect) const
{
	if (const FSekiroAttackContext* Ctx = ConsumedEffect.GetPtr<FSekiroAttackContext>())
	{
		return Ctx->bIsInAir;
	}
	return false;
}
