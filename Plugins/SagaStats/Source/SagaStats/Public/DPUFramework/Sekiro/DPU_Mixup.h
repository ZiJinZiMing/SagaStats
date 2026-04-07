// DPU_Mixup.h — 猜拳判定机制（Fact + ConditionNode + Logic）
#pragma once

#include "CoreMinimal.h"
#include "StructUtils/InstancedStruct.h"
#include "DPUFramework/DPULogicBase.h"
#include "DPUFramework/DPUCondition.h"
#include "DPUFramework/DamageContext.h"
#include "DPU_Mixup.generated.h"

// ============================================================================
// Fact
// ============================================================================

USTRUCT(BlueprintType)
struct SAGASTATS_API FMixupResult
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
class SAGASTATS_API UDPUCondition_IsGuard : public UDPUCondition
{
	GENERATED_BODY()
public:
	virtual UScriptStruct* GetConsumedFactType() const override { return FMixupResult::StaticStruct(); }
	virtual bool Evaluate_Implementation(const UDamageContext* DC, const FInstancedStruct& ConsumedFact) const override;
};

UCLASS(BlueprintType, meta = (DisplayName = "IsJustGuard"))
class SAGASTATS_API UDPUCondition_IsJustGuard : public UDPUCondition
{
	GENERATED_BODY()
public:
	virtual UScriptStruct* GetConsumedFactType() const override { return FMixupResult::StaticStruct(); }
	virtual bool Evaluate_Implementation(const UDamageContext* DC, const FInstancedStruct& ConsumedFact) const override;
};

// ============================================================================
// Logic
// ============================================================================

UCLASS()
class SAGASTATS_API UDPULogic_Mixup : public UDPULogicBase
{
	GENERATED_BODY()
public:
	virtual void Execute_Implementation(UDamageContext* DC, FInstancedStruct& OutFact) override;
	virtual UScriptStruct* GetProducesFactType() const override { return FMixupResult::StaticStruct(); }
};
