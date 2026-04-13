// DR_CollapseJustGuard.cpp — 崩溃弹反信号实现
#include "DamageFramework/Sekiro/DR_CollapseJustGuard.h"

// ============================================================================
// Operation
// ============================================================================

void UDamageOperation_CollapseJustGuard::Execute_Implementation(UDamageContext* Context, FInstancedStruct& OutEffect)
{
	OutEffect = FInstancedStruct::Make<FCollapseJustGuardEffect>(FCollapseJustGuardEffect{});
}
