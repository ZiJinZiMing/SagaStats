/***************************************************************************************************************
* Plugin:       SagaStats
* Author:       Jinming Zhang
* Description:  SagaStats offers modular damage process and meter systems to support adaptable status management
****************************************************************************************************************/

// DR_CollapseJustGuard.h — 崩溃弹反信号（Effect + Condition + Operation）
#pragma once

#include "CoreMinimal.h"
#include "StructUtils/InstancedStruct.h"
#include "DamagePipeline/DamageOperationBase.h"
#include "DamagePipeline/DamageCondition_Effect.h"
#include "DamagePipeline/DamageContext.h"
#include "DR_CollapseJustGuard.generated.h"

// ============================================================================
// Effect（信号型，无字段）
// ============================================================================

USTRUCT(BlueprintType)
struct SAGASTATS_API FCollapseJustGuardEffect
{
	GENERATED_BODY()
};

// ============================================================================
// Condition
// ============================================================================

UCLASS(BlueprintType, HideDropDown, meta = (DisplayName = "CollapseJustGuard"))
class SAGASTATS_API UDamageCondition_CollapseJustGuard : public UDamageCondition_Effect
{
	GENERATED_BODY()
public:
	UDamageCondition_CollapseJustGuard() { EffectType = FCollapseJustGuardEffect::StaticStruct(); }
	virtual bool Evaluate_Implementation(const UDamageContext* Context, const FInstancedStruct& ConsumedEffect) const override { return ConsumedEffect.IsValid(); }
};

// ============================================================================
// Operation
// ============================================================================

UCLASS(HideDropDown)
class SAGASTATS_API UDamageOperation_CollapseJustGuard : public UDamageOperationBase
{
	GENERATED_BODY()
public:
	UDamageOperation_CollapseJustGuard() { EffectType = FCollapseJustGuardEffect::StaticStruct(); }
	virtual void Execute_Implementation(UDamageContext* Context, FInstancedStruct& OutEffect) override;
};
