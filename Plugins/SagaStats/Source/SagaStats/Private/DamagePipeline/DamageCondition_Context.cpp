/***************************************************************************************************************
* Plugin:       SagaStats
* Author:       Jinming Zhang
* Description:  SagaStats offers modular damage process and meter systems to support adaptable status management
****************************************************************************************************************/

// DamageCondition_Context.cpp — 基于 Context 的条件原子实现（直接 dispatch）
#include "DamagePipeline/DamageCondition_Context.h"

bool UDamageCondition_Context::EvaluateCondition(const UDamageContext* Context) const
{
	return Evaluate(Context);
}
