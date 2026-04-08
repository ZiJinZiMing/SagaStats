// DR_LightningOnGround.cpp — 地面雷信号实现
#include "DamageFramework/Sekiro/DR_LightningOnGround.h"

// ============================================================================
// Logic
// ============================================================================

void UDamageOperation_LightningOnGround::Execute_Implementation(UDamageContext* Context, FInstancedStruct& OutEffect)
{
	OutEffect = FInstancedStruct::Make<FLightningOnGroundEffect>(FLightningOnGroundEffect{});
}
