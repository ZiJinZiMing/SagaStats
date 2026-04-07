// DPU_Toughness.cpp — 韧性信号实现
#include "DPUFramework/Sekiro/DPU_Toughness.h"

// ============================================================================
// Logic
// ============================================================================

void UDPULogic_Toughness::Execute_Implementation(UDamageContext* DC, FInstancedStruct& OutFact)
{
	OutFact = FInstancedStruct::Make<FToughnessSignal>(FToughnessSignal{});
}
