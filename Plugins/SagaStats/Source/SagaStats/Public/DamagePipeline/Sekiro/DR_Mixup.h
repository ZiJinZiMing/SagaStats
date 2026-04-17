/***************************************************************************************************************
* Plugin:       SagaStats
* Author:       Jinming Zhang
* Description:  SagaStats offers modular damage process and meter systems to support adaptable status management
****************************************************************************************************************/

// DR_Mixup.h — 猜拳判定机制（Effect + Condition + Operation）
#pragma once

#include "CoreMinimal.h"
#include "StructUtils/InstancedStruct.h"
#include "DamagePipeline/DamageOperationBase.h"
#include "DamagePipeline/DamageCondition_Effect.h"
#include "DamagePipeline/DamageContext.h"
#include "DR_Mixup.generated.h"

// ============================================================================
// Effect
// ============================================================================

USTRUCT(BlueprintType)
struct SAGASTATS_API FMixupEffect
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsGuard = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsJustGuard = false;
};

// ============================================================================
// Condition
// ============================================================================

UCLASS(BlueprintType, HideDropDown, meta = (DisplayName = "IsGuard"))
class SAGASTATS_API UDamageCondition_IsGuard : public UDamageCondition_Effect
{
	GENERATED_BODY()
public:
	UDamageCondition_IsGuard() { EffectType = FMixupEffect::StaticStruct(); }
	virtual bool Evaluate_Implementation(const UDamageContext* Context, const FInstancedStruct& ConsumedEffect) const override;
};

UCLASS(BlueprintType, HideDropDown, meta = (DisplayName = "IsJustGuard"))
class SAGASTATS_API UDamageCondition_IsJustGuard : public UDamageCondition_Effect
{
	GENERATED_BODY()
public:
	UDamageCondition_IsJustGuard() { EffectType = FMixupEffect::StaticStruct(); }
	virtual bool Evaluate_Implementation(const UDamageContext* Context, const FInstancedStruct& ConsumedEffect) const override;
};

// ============================================================================
// Operation
// ============================================================================

UCLASS(HideDropDown)
class SAGASTATS_API UDamageOperation_Mixup : public UDamageOperationBase
{
	GENERATED_BODY()
public:
	UDamageOperation_Mixup() { EffectType = FMixupEffect::StaticStruct(); }
	virtual void Execute_Implementation(UDamageContext* Context, FInstancedStruct& OutEffect) override;
};
