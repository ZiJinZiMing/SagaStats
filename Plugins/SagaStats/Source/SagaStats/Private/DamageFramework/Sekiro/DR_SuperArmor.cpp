// DR_SuperArmor.cpp — 超级装甲信号实现
#include "DamageFramework/Sekiro/DR_SuperArmor.h"

// ============================================================================
// Operation
// ============================================================================

void UDamageOperation_SuperArmor::Execute_Implementation(UDamageContext* Context, FInstancedStruct& OutEffect)
{
	OutEffect = FInstancedStruct::Make<FSuperArmorEffect>(FSuperArmorEffect{});
}
