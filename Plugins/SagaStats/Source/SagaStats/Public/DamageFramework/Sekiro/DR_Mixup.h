// DR_Mixup.h — 猜拳判定机制（Effect + Condition + Operation）
#pragma once

#include "CoreMinimal.h"
#include "StructUtils/InstancedStruct.h"
#include "DamageFramework/DamageOperationBase.h"
#include "DamageFramework/DamageCondition.h"
#include "DamageFramework/DamageContext.h"
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

UCLASS(BlueprintType, meta = (DisplayName = "IsGuard"))
class SAGASTATS_API UDamageCondition_IsGuard : public UDamageCondition
{
	GENERATED_BODY()
public:
	virtual UScriptStruct* GetConsumedEffectType() const override { return FMixupEffect::StaticStruct(); }
	virtual bool Evaluate_Implementation(const UDamageContext* Context, const FInstancedStruct& ConsumedFact) const override;
};

UCLASS(BlueprintType, meta = (DisplayName = "IsJustGuard"))
class SAGASTATS_API UDamageCondition_IsJustGuard : public UDamageCondition
{
	GENERATED_BODY()
public:
	virtual UScriptStruct* GetConsumedEffectType() const override { return FMixupEffect::StaticStruct(); }
	virtual bool Evaluate_Implementation(const UDamageContext* Context, const FInstancedStruct& ConsumedFact) const override;
};

// ============================================================================
// Logic
// ============================================================================

UCLASS()
class SAGASTATS_API UDamageOperation_Mixup : public UDamageOperationBase
{
	GENERATED_BODY()
public:
	virtual void Execute_Implementation(UDamageContext* Context, FInstancedStruct& OutEffect) override;
	virtual UScriptStruct* GetProducesEffectType() const override { return FMixupEffect::StaticStruct(); }
};
