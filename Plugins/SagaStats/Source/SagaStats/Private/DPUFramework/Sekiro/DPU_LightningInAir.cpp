// DPU_LightningInAir.cpp — 空中雷信号实现
#include "DPUFramework/Sekiro/DPU_LightningInAir.h"

// ============================================================================
// Logic
// ============================================================================

void UDPULogic_LightningInAir::Execute_Implementation(UDamageContext* DC, FInstancedStruct& OutFact)
{
	OutFact = FInstancedStruct::Make<FLightningInAirSignal>(FLightningInAirSignal{});
}
