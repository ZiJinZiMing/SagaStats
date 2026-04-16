/***************************************************************************************************************
* Plugin:       SagaStats
* Author:       Jinming Zhang
* Description:  SagaStats offers modular damage process and meter systems to support adaptable status management
****************************************************************************************************************/

// DR_SuperArmor.cpp — 超级装甲信号实现
#include "DamagePipeline/Sekiro/DR_SuperArmor.h"

// ============================================================================
// Operation
// ============================================================================

void UDamageOperation_SuperArmor::Execute_Implementation(UDamageContext* Context, FInstancedStruct& OutEffect)
{
	OutEffect = FInstancedStruct::Make<FSuperArmorEffect>(FSuperArmorEffect{});
}
