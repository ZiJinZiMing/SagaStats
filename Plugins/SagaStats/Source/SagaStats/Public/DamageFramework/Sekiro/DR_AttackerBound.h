// DR_AttackerBound.h — 攻击者绑定机制（Effect + Condition + Operation）
#pragma once

#include "CoreMinimal.h"
#include "StructUtils/InstancedStruct.h"
#include "DamageFramework/DamageOperationBase.h"
#include "DamageFramework/DamageCondition.h"
#include "DamageFramework/DamageContext.h"
#include "DR_AttackerBound.generated.h"

// ============================================================================
// Effect
// ============================================================================

USTRUCT(BlueprintType)
struct SAGASTATS_API FAttackerBoundEffect
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsBound = false;
};

// ============================================================================
// Condition
// ============================================================================

UCLASS(BlueprintType, HideDropDown, meta = (DisplayName = "IsBound"))
class SAGASTATS_API UDamageCondition_IsBound : public UDamageCondition
{
	GENERATED_BODY()
public:
	virtual UScriptStruct* GetConsumedEffectType() const override { return FAttackerBoundEffect::StaticStruct(); }
	virtual bool Evaluate_Implementation(const UDamageContext* Context, const FInstancedStruct& ConsumedEffect) const override;
};

// ============================================================================
// Logic
// ============================================================================

UCLASS(HideDropDown)
class SAGASTATS_API UDamageOperation_AttackerBound : public UDamageOperationBase
{
	GENERATED_BODY()
public:
	virtual void Execute_Implementation(UDamageContext* Context, FInstancedStruct& OutEffect) override;
	virtual UScriptStruct* GetProducesEffectType() const override { return FAttackerBoundEffect::StaticStruct(); }
};
