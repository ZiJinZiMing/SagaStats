/***************************************************************************************************************
* Plugin:       SagaStats
* Author:       Jinming Zhang
* Description:  SagaStats offers modular damage process and meter systems to support adaptable status management
****************************************************************************************************************/

// DR_LightningOnGround.cpp — 地面雷信号实现
#include "DamagePipeline/Sekiro/DR_LightningOnGround.h"

// ============================================================================
// Operation
// ============================================================================

void UDamageOperation_LightningOnGround::Execute_Implementation(UDamageContext* Context, FInstancedStruct& OutEffect)
{
	OutEffect = FInstancedStruct::Make<FLightningOnGroundEffect>(FLightningOnGroundEffect{});
}
