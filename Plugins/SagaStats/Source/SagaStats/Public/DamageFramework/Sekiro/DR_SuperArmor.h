// DR_SuperArmor.h — 超级装甲信号（Effect + Condition + Operation）
#pragma once

#include "CoreMinimal.h"
#include "StructUtils/InstancedStruct.h"
#include "DamageFramework/DamageOperationBase.h"
#include "DamageFramework/DamageCondition.h"
#include "DamageFramework/DamageContext.h"
#include "DR_SuperArmor.generated.h"

// ============================================================================
// Effect（信号型，无字段）
// ============================================================================

USTRUCT(BlueprintType)
struct SAGASTATS_API FSuperArmorEffect
{
	GENERATED_BODY()
};

// ============================================================================
// Condition
// ============================================================================

UCLASS(BlueprintType, HideDropDown, meta = (DisplayName = "SuperArmor"))
class SAGASTATS_API UDamageCondition_SuperArmor : public UDamageCondition
{
	GENERATED_BODY()
public:
	virtual UScriptStruct* GetConsumedEffectType() const override { return FSuperArmorEffect::StaticStruct(); }
	virtual bool Evaluate_Implementation(const UDamageContext* Context, const FInstancedStruct& ConsumedEffect) const override { return ConsumedEffect.IsValid(); }
};

// ============================================================================
// Logic
// ============================================================================

UCLASS(HideDropDown)
class SAGASTATS_API UDamageOperation_SuperArmor : public UDamageOperationBase
{
	GENERATED_BODY()
public:
	virtual void Execute_Implementation(UDamageContext* Context, FInstancedStruct& OutEffect) override;
	virtual UScriptStruct* GetProducesEffectType() const override { return FSuperArmorEffect::StaticStruct(); }
};
