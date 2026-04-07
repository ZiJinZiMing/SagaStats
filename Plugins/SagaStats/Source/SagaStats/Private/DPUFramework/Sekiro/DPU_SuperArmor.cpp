// DPU_SuperArmor.cpp — 超级装甲信号实现
#include "DPUFramework/Sekiro/DPU_SuperArmor.h"

// ============================================================================
// Logic
// ============================================================================

void UDPULogic_SuperArmor::Execute_Implementation(UDamageContext* DC, FInstancedStruct& OutFact)
{
	OutFact = FInstancedStruct::Make<FSuperArmorSignal>(FSuperArmorSignal{});
}
