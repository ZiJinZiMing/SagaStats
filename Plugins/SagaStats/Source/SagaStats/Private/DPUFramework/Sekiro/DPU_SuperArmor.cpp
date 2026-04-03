// DPU_SuperArmor.cpp — 超级装甲信号实现
#include "DPUFramework/Sekiro/DPU_SuperArmor.h"

// ============================================================================
// Logic
// ============================================================================

FInstancedStruct UDPULogic_SuperArmor::Execute_Implementation(const UDamageContext* DC)
{
	return FInstancedStruct::Make<FSuperArmorSignal>(FSuperArmorSignal{});
}
