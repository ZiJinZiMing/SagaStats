// DPU_LightningInAir.h — 空中雷信号（Fact + ConditionNode + Logic）
#pragma once

#include "CoreMinimal.h"
#include "StructUtils/InstancedStruct.h"
#include "DPUFramework/DPULogicBase.h"
#include "DPUFramework/ConditionNode.h"
#include "DPUFramework/DamageContext.h"
#include "DPU_LightningInAir.generated.h"

// ============================================================================
// Fact（信号型，无字段）
// ============================================================================

USTRUCT(BlueprintType)
struct SAGASTATS_API FLightningInAirSignal
{
	GENERATED_BODY()
};

// ============================================================================
// ConditionNode
// ============================================================================

UCLASS(BlueprintType, meta = (DisplayName = "LightningInAir"))
class SAGASTATS_API UConditionNode_LightningInAir : public UConditionNode_DPUBase
{
	GENERATED_BODY()
public:
	virtual UScriptStruct* GetConsumedFactType() const override { return FLightningInAirSignal::StaticStruct(); }
};

// ============================================================================
// Logic
// ============================================================================

UCLASS()
class SAGASTATS_API UDPULogic_LightningInAir : public UDPULogicBase
{
	GENERATED_BODY()
public:
	virtual FInstancedStruct Execute_Implementation(const UDamageContext* DC) override;
};
