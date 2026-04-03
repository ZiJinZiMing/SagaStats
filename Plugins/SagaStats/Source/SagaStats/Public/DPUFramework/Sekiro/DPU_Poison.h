// DPU_Poison.h — 中毒判定机制（Fact + ConditionNode + Logic）
#pragma once

#include "CoreMinimal.h"
#include "StructUtils/InstancedStruct.h"
#include "DPUFramework/DPULogicBase.h"
#include "DPUFramework/ConditionNode.h"
#include "DPUFramework/DamageContext.h"
#include "DPU_Poison.generated.h"

// ============================================================================
// Fact
// ============================================================================

USTRUCT(BlueprintType)
struct SAGASTATS_API FPoisonResult
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsPoisoned = false;
};

// ============================================================================
// ConditionNode
// ============================================================================

UCLASS(BlueprintType, meta = (DisplayName = "Poison"))
class SAGASTATS_API UConditionNode_Poison : public UConditionNode_DPUBase
{
	GENERATED_BODY()
public:
	virtual UScriptStruct* GetConsumedFactType() const override { return FPoisonResult::StaticStruct(); }

	UFUNCTION() bool IsPoisoned(const UDamageContext* DC) const;
};

// ============================================================================
// Logic
// ============================================================================

UCLASS()
class SAGASTATS_API UDPULogic_Poison : public UDPULogicBase
{
	GENERATED_BODY()
public:
	virtual FInstancedStruct Execute_Implementation(const UDamageContext* DC) override;
};
