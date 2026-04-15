// DR_Collapse.h — 崩架势机制（Collapse + CollapseGuard，各自独立 Effect 类型）
#pragma once

#include "CoreMinimal.h"
#include "StructUtils/InstancedStruct.h"
#include "DamageFramework/DamageOperationBase.h"
#include "DamageFramework/DamageCondition.h"
#include "DamageFramework/DamageContext.h"
#include "DR_Collapse.generated.h"

// ============================================================================
// Effect — 每个 DamageRule 独立的 Effect 类型（DamageRule 和 Effect 一一对应）
// ============================================================================

USTRUCT(BlueprintType)
struct SAGASTATS_API FCollapseEffect
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsCollapse = false;
};

USTRUCT(BlueprintType)
struct SAGASTATS_API FCollapseGuardEffect
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsCollapse = false;
};

// ============================================================================
// Condition — Collapse
// ============================================================================

UCLASS(BlueprintType, HideDropDown, meta = (DisplayName = "CollapseIsCollapse"))
class SAGASTATS_API UDamageCondition_CollapseIsCollapse : public UDamageCondition
{
	GENERATED_BODY()
public:
	virtual UScriptStruct* GetEffectType() const override { return FCollapseEffect::StaticStruct(); }
	virtual bool Evaluate_Implementation(const UDamageContext* Context, const FInstancedStruct& ConsumedEffect) const override;
};

// ============================================================================
// Condition — CollapseGuard
// ============================================================================

UCLASS(BlueprintType, HideDropDown, meta = (DisplayName = "CollapseGuardIsCollapse"))
class SAGASTATS_API UDamageCondition_CollapseGuardIsCollapse : public UDamageCondition
{
	GENERATED_BODY()
public:
	virtual UScriptStruct* GetEffectType() const override { return FCollapseGuardEffect::StaticStruct(); }
	virtual bool Evaluate_Implementation(const UDamageContext* Context, const FInstancedStruct& ConsumedEffect) const override;
};

// ============================================================================
// Operation
// ============================================================================

UCLASS(HideDropDown)
class SAGASTATS_API UDamageOperation_Collapse : public UDamageOperationBase
{
	GENERATED_BODY()
public:
	virtual void Execute_Implementation(UDamageContext* Context, FInstancedStruct& OutEffect) override;
	virtual UScriptStruct* GetEffectType() const override { return FCollapseEffect::StaticStruct(); }
};

UCLASS(HideDropDown)
class SAGASTATS_API UDamageOperation_CollapseGuard : public UDamageOperationBase
{
	GENERATED_BODY()
public:
	virtual void Execute_Implementation(UDamageContext* Context, FInstancedStruct& OutEffect) override;
	virtual UScriptStruct* GetEffectType() const override { return FCollapseGuardEffect::StaticStruct(); }
};
