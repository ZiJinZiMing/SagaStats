// DR_Toughness.cpp — 韧性信号实现
#include "DamageFramework/Sekiro/DR_Toughness.h"

// ============================================================================
// Logic
// ============================================================================

void UDamageOperation_Toughness::Execute_Implementation(UDamageContext* Context, FInstancedStruct& OutEffect)
{
	OutEffect = FInstancedStruct::Make<FToughnessEffect>(FToughnessEffect{});
}
