// DPU_LightningOnGround.cpp — 地面雷信号实现
#include "DPUFramework/Sekiro/DPU_LightningOnGround.h"

// ============================================================================
// Logic
// ============================================================================

void UDPULogic_LightningOnGround::Execute_Implementation(UDamageContext* DC, FInstancedStruct& OutFact)
{
	OutFact = FInstancedStruct::Make<FLightningOnGroundSignal>(FLightningOnGroundSignal{});
}
