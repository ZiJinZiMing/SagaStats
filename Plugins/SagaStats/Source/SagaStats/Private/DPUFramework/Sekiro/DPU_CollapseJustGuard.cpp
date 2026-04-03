// DPU_CollapseJustGuard.cpp — 崩溃弹反信号实现
#include "DPUFramework/Sekiro/DPU_CollapseJustGuard.h"

// ============================================================================
// Logic
// ============================================================================

FInstancedStruct UDPULogic_CollapseJustGuard::Execute_Implementation(const UDamageContext* DC)
{
	return FInstancedStruct::Make<FCollapseJustGuardSignal>(FCollapseJustGuardSignal{});
}
