/***************************************************************************************************************
* Plugin:       SagaStats
* Author:       Jinming Zhang
* Description:  SagaStats offers modular damage process and meter systems to support adaptable status management
****************************************************************************************************************/

// DR_Guard.h — 防御判定机制（Effect + Condition + Operation）
#pragma once

#include "CoreMinimal.h"
#include "StructUtils/InstancedStruct.h"
#include "DamagePipeline/DamageOperationBase.h"
#include "DamagePipeline/DamageCondition_Effect.h"
#include "DamagePipeline/DamageContext.h"
#include "DR_Guard.generated.h"

// ============================================================================
// Effect
// ============================================================================

USTRUCT(BlueprintType)
struct SAGASTATS_API FGuardEffect
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bGuardSuccess = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsJustGuard = false;
};

// ============================================================================
// Condition
// ============================================================================

UCLASS(BlueprintType, HideDropDown, meta = (DisplayName = "GuardSuccess"))
class SAGASTATS_API UDamageCondition_GuardSuccess : public UDamageCondition_Effect
{
	GENERATED_BODY()
public:
	UDamageCondition_GuardSuccess() { EffectType = FGuardEffect::StaticStruct(); }
	virtual bool Evaluate_Implementation(const UDamageContext* Context, const FInstancedStruct& ConsumedEffect) const override;
};

UCLASS(BlueprintType, HideDropDown, meta = (DisplayName = "GuardIsJustGuard"))
class SAGASTATS_API UDamageCondition_GuardIsJustGuard : public UDamageCondition_Effect
{
	GENERATED_BODY()
public:
	UDamageCondition_GuardIsJustGuard() { EffectType = FGuardEffect::StaticStruct(); }
	virtual bool Evaluate_Implementation(const UDamageContext* Context, const FInstancedStruct& ConsumedEffect) const override;
};

// ============================================================================
// Operation
// ============================================================================

UCLASS(HideDropDown)
class SAGASTATS_API UDamageOperation_Guard : public UDamageOperationBase
{
	GENERATED_BODY()
public:
	UDamageOperation_Guard() { EffectType = FGuardEffect::StaticStruct(); }
	virtual void Execute_Implementation(UDamageContext* Context, FInstancedStruct& OutEffect) override;
};
