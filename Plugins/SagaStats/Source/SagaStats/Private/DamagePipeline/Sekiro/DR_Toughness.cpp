/***************************************************************************************************************
* Plugin:       SagaStats
* Author:       Jinming Zhang
* Description:  SagaStats offers modular damage process and meter systems to support adaptable status management
****************************************************************************************************************/

// DR_Toughness.cpp — 韧性信号实现
#include "DamagePipeline/Sekiro/DR_Toughness.h"

// ============================================================================
// Operation
// ============================================================================

void UDamageOperation_Toughness::Execute_Implementation(UDamageContext* Context, FInstancedStruct& OutEffect)
{
	OutEffect = FInstancedStruct::Make<FToughnessEffect>(FToughnessEffect{});
}
