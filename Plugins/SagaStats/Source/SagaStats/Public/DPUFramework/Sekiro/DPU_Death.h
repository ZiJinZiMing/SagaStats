// DPU_Death.h — 死亡判定机制（Fact + ConditionNode + Logic）
#pragma once

#include "CoreMinimal.h"
#include "StructUtils/InstancedStruct.h"
#include "DPUFramework/DPULogicBase.h"
#include "DPUFramework/ConditionNode.h"
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
// ConditionNode
// ============================================================================

UCLASS(BlueprintType, meta = (DisplayName = "Death"))
class SAGASTATS_API UConditionNode_Death : public UConditionNode_DPUBase
{
	GENERATED_BODY()
public:
	virtual UScriptStruct* GetConsumedFactType() const override { return FDeathResult::StaticStruct(); }

	UFUNCTION() bool IsDead(const UDamageContext* DC) const;
};

// ============================================================================
// Logic
// ============================================================================

UCLASS()
class SAGASTATS_API UDPULogic_Death : public UDPULogicBase
{
	GENERATED_BODY()
public:
	virtual FInstancedStruct Execute_Implementation(const UDamageContext* DC) override;
};
