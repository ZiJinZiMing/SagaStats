// DPU_LightningOnGround.cpp — 地面雷信号实现
#include "DPUFramework/Sekiro/DPU_LightningOnGround.h"

// ============================================================================
// Logic
// ============================================================================

FInstancedStruct UDPULogic_LightningOnGround::Execute_Implementation(const UDamageContext* DC)
{
	return FInstancedStruct::Make<FLightningOnGroundSignal>(FLightningOnGroundSignal{});
}
