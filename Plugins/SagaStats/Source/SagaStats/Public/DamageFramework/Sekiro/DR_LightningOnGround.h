// DR_LightningOnGround.h — 地面雷信号（Effect + Condition + Operation）
#pragma once

#include "CoreMinimal.h"
#include "StructUtils/InstancedStruct.h"
#include "DamageFramework/DamageOperationBase.h"
#include "DamageFramework/DamageCondition.h"
#include "DamageFramework/DamageContext.h"
#include "DR_LightningOnGround.generated.h"

// ============================================================================
// Effect（信号型，无字段）
// ============================================================================

USTRUCT(BlueprintType)
struct SAGASTATS_API FLightningOnGroundEffect
{
	GENERATED_BODY()
};

// ============================================================================
// Condition
// ============================================================================

UCLASS(BlueprintType, meta = (DisplayName = "LightningOnGround"))
class SAGASTATS_API UDamageCondition_LightningOnGround : public UDamageCondition
{
	GENERATED_BODY()
public:
	virtual UScriptStruct* GetConsumedEffectType() const override { return FLightningOnGroundEffect::StaticStruct(); }
	virtual bool Evaluate_Implementation(const UDamageContext* Context, const FInstancedStruct& ConsumedFact) const override { return ConsumedFact.IsValid(); }
};

// ============================================================================
// Logic
// ============================================================================

UCLASS()
class SAGASTATS_API UDamageOperation_LightningOnGround : public UDamageOperationBase
{
	GENERATED_BODY()
public:
	virtual void Execute_Implementation(UDamageContext* Context, FInstancedStruct& OutEffect) override;
	virtual UScriptStruct* GetProducesEffectType() const override { return FLightningOnGroundEffect::StaticStruct(); }
};
