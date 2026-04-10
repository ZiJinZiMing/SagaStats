// DR_Death.h — 死亡判定机制（Effect + Condition + Operation）
#pragma once

#include "CoreMinimal.h"
#include "StructUtils/InstancedStruct.h"
#include "DamageFramework/DamageOperationBase.h"
#include "DamageFramework/DamageCondition.h"
#include "DamageFramework/DamageContext.h"
#include "DR_Death.generated.h"

// ============================================================================
// Effect
// ============================================================================

USTRUCT(BlueprintType)
struct SAGASTATS_API FDeathEffect
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsDead = false;
};

// ============================================================================
// Condition
// ============================================================================

UCLASS(BlueprintType, HideDropDown, meta = (DisplayName = "IsDead"))
class SAGASTATS_API UDamageCondition_IsDead : public UDamageCondition
{
	GENERATED_BODY()
public:
	virtual UScriptStruct* GetConsumedEffectType() const override { return FDeathEffect::StaticStruct(); }
	virtual bool Evaluate_Implementation(const UDamageContext* Context, const FInstancedStruct& ConsumedEffect) const override;
};

// ============================================================================
// Logic
// ============================================================================

UCLASS(HideDropDown)
class SAGASTATS_API UDamageOperation_Death : public UDamageOperationBase
{
	GENERATED_BODY()
public:
	virtual void Execute_Implementation(UDamageContext* Context, FInstancedStruct& OutEffect) override;
	virtual UScriptStruct* GetProducesEffectType() const override { return FDeathEffect::StaticStruct(); }
};
