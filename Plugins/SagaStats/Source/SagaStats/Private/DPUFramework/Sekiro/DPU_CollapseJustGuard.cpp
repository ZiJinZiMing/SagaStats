// DPU_CollapseJustGuard.cpp — 崩溃弹反信号实现
#include "DPUFramework/Sekiro/DPU_CollapseJustGuard.h"

// ============================================================================
// Logic
// ============================================================================

void UDPULogic_CollapseJustGuard::Execute_Implementation(UDamageContext* DC, FInstancedStruct& OutFact)
{
	OutFact = FInstancedStruct::Make<FCollapseJustGuardSignal>(FCollapseJustGuardSignal{});
}
