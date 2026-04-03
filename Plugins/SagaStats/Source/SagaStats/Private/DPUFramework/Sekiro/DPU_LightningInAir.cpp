// DPU_LightningInAir.cpp — 空中雷信号实现
#include "DPUFramework/Sekiro/DPU_LightningInAir.h"

// ============================================================================
// Logic
// ============================================================================

FInstancedStruct UDPULogic_LightningInAir::Execute_Implementation(const UDamageContext* DC)
{
	return FInstancedStruct::Make<FLightningInAirSignal>(FLightningInAirSignal{});
}
