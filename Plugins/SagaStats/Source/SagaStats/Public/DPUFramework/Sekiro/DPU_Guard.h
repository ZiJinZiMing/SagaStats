// DPU_Guard.h — 防御判定机制（Fact + ConditionNode + Logic）
#pragma once

#include "CoreMinimal.h"
#include "StructUtils/InstancedStruct.h"
#include "DPUFramework/DPULogicBase.h"
#include "DPUFramework/DPUCondition.h"
#include "DPUFramework/DamageContext.h"
#include "DPU_Guard.generated.h"

// ============================================================================
// Fact
// ============================================================================

USTRUCT(BlueprintType)
struct SAGASTATS_API FGuardResult
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

UCLASS(BlueprintType, meta = (DisplayName = "GuardSuccess"))
class SAGASTATS_API UDPUCondition_GuardSuccess : public UDPUCondition
{
	GENERATED_BODY()
public:
	virtual UScriptStruct* GetConsumedFactType() const override { return FGuardResult::StaticStruct(); }
	virtual bool Evaluate_Implementation(const UDamageContext* DC, const FInstancedStruct& ConsumedFact) const override;
};

UCLASS(BlueprintType, meta = (DisplayName = "GuardIsJustGuard"))
class SAGASTATS_API UDPUCondition_GuardIsJustGuard : public UDPUCondition
{
	GENERATED_BODY()
public:
	virtual UScriptStruct* GetConsumedFactType() const override { return FGuardResult::StaticStruct(); }
	virtual bool Evaluate_Implementation(const UDamageContext* DC, const FInstancedStruct& ConsumedFact) const override;
};

// ============================================================================
// Logic
// ============================================================================

UCLASS()
class SAGASTATS_API UDPULogic_Guard : public UDPULogicBase
{
	GENERATED_BODY()
public:
	virtual void Execute_Implementation(UDamageContext* DC, FInstancedStruct& OutFact) override;
	virtual UScriptStruct* GetProducesFactType() const override { return FGuardResult::StaticStruct(); }
};
