// DR_LightningInAir.h — 空中雷信号（Effect + Condition + Operation）
#pragma once

#include "CoreMinimal.h"
#include "StructUtils/InstancedStruct.h"
#include "DamageFramework/DamageOperationBase.h"
#include "DamageFramework/DamageCondition.h"
#include "DamageFramework/DamageContext.h"
#include "DR_LightningInAir.generated.h"

// ============================================================================
// Effect（信号型，无字段）
// ============================================================================

USTRUCT(BlueprintType)
struct SAGASTATS_API FLightningInAirEffect
{
	GENERATED_BODY()
};

// ============================================================================
// Condition
// ============================================================================

UCLASS(BlueprintType, meta = (DisplayName = "LightningInAir"))
class SAGASTATS_API UDamageCondition_LightningInAir : public UDamageCondition
{
	GENERATED_BODY()
public:
	virtual UScriptStruct* GetConsumedEffectType() const override { return FLightningInAirEffect::StaticStruct(); }
	virtual bool Evaluate_Implementation(const UDamageContext* Context, const FInstancedStruct& ConsumedFact) const override { return ConsumedFact.IsValid(); }
};

// ============================================================================
// Logic
// ============================================================================

UCLASS()
class SAGASTATS_API UDamageOperation_LightningInAir : public UDamageOperationBase
{
	GENERATED_BODY()
public:
	virtual void Execute_Implementation(UDamageContext* Context, FInstancedStruct& OutEffect) override;
	virtual UScriptStruct* GetProducesEffectType() const override { return FLightningInAirEffect::StaticStruct(); }
};
