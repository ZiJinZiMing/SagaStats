// DPU_Toughness.cpp — 韧性信号实现
#include "DPUFramework/Sekiro/DPU_Toughness.h"

// ============================================================================
// Logic
// ============================================================================

FInstancedStruct UDPULogic_Toughness::Execute_Implementation(const UDamageContext* DC)
{
	return FInstancedStruct::Make<FToughnessSignal>(FToughnessSignal{});
}
