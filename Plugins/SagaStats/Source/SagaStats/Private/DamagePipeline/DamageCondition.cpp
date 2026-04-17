/***************************************************************************************************************
* Plugin:       SagaStats
* Author:       Jinming Zhang
* Description:  SagaStats offers modular damage process and meter systems to support adaptable status management
****************************************************************************************************************/

// DamageCondition.cpp — 条件原子抽象基类（实现空——入口逻辑在 _Effect/_Context 子类）
#include "DamagePipeline/DamageCondition.h"

// 基类为 Abstract + PURE_VIRTUAL，无 EvaluateCondition 实现。
// 预取和 dispatch 逻辑分别在：
//   DamageCondition_Effect.cpp  —— 按 EffectType 预取 Effect 后调 Evaluate(Ctx, InEffect)
//   DamageCondition_Context.cpp —— 直接调 Evaluate(Ctx)
