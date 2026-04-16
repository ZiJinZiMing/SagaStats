/***************************************************************************************************************
* Plugin:       SagaStats
* Author:       Jinming Zhang
* Description:  SagaStats offers modular damage process and meter systems to support adaptable status management
****************************************************************************************************************/

// DR_CollapseJustGuard.cpp — 崩溃弹反信号实现
#include "DamagePipeline/Sekiro/DR_CollapseJustGuard.h"

// ============================================================================
// Operation
// ============================================================================

void UDamageOperation_CollapseJustGuard::Execute_Implementation(UDamageContext* Context, FInstancedStruct& OutEffect)
{
	OutEffect = FInstancedStruct::Make<FCollapseJustGuardEffect>(FCollapseJustGuardEffect{});
}
