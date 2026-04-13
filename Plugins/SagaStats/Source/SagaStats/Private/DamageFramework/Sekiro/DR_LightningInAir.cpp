// DR_LightningInAir.cpp — 空中雷信号实现
#include "DamageFramework/Sekiro/DR_LightningInAir.h"

// ============================================================================
// Operation
// ============================================================================

void UDamageOperation_LightningInAir::Execute_Implementation(UDamageContext* Context, FInstancedStruct& OutEffect)
{
	OutEffect = FInstancedStruct::Make<FLightningInAirEffect>(FLightningInAirEffect{});
}
