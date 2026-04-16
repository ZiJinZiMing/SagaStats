/***************************************************************************************************************
* Plugin:       SagaStats
* Author:       Jinming Zhang
* Description:  SagaStats offers modular damage process and meter systems to support adaptable status management
****************************************************************************************************************/

// DR_LightningInAir.cpp — 空中雷信号实现
#include "DamagePipeline/Sekiro/DR_LightningInAir.h"

// ============================================================================
// Operation
// ============================================================================

void UDamageOperation_LightningInAir::Execute_Implementation(UDamageContext* Context, FInstancedStruct& OutEffect)
{
	OutEffect = FInstancedStruct::Make<FLightningInAirEffect>(FLightningInAirEffect{});
}
