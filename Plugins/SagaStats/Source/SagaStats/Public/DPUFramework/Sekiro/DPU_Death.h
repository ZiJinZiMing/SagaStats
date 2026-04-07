// DPU_Death.h — 死亡判定机制（Fact + ConditionNode + Logic）
#pragma once

#include "CoreMinimal.h"
#include "StructUtils/InstancedStruct.h"
#include "DPUFramework/DPULogicBase.h"
#include "DPUFramework/DPUCondition.h"
#include "DPUFramework/DamageContext.h"
#include "DPU_Death.generated.h"

// ============================================================================
// Fact
// ============================================================================

USTRUCT(BlueprintType)
struct SAGASTATS_API FDeathResult
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsDead = false;
};

// ============================================================================
// Condition
// ============================================================================

UCLASS(BlueprintType, meta = (DisplayName = "IsDead"))
class SAGASTATS_API UDPUCondition_IsDead : public UDPUCondition
{
	GENERATED_BODY()
public:
	virtual UScriptStruct* GetConsumedFactType() const override { return FDeathResult::StaticStruct(); }
	virtual bool Evaluate_Implementation(const UDamageContext* DC, const FInstancedStruct& ConsumedFact) const override;
};

// ============================================================================
// Logic
// ============================================================================

UCLASS()
class SAGASTATS_API UDPULogic_Death : public UDPULogicBase
{
	GENERATED_BODY()
public:
	virtual void Execute_Implementation(UDamageContext* DC, FInstancedStruct& OutFact) override;
	virtual UScriptStruct* GetProducesFactType() const override { return FDeathResult::StaticStruct(); }
};
