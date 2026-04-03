// DPU_LightningOnGround.h — 地面雷信号（Fact + ConditionNode + Logic）
#pragma once

#include "CoreMinimal.h"
#include "StructUtils/InstancedStruct.h"
#include "DPUFramework/DPULogicBase.h"
#include "DPUFramework/ConditionNode.h"
#include "DPUFramework/DamageContext.h"
#include "DPU_LightningOnGround.generated.h"

// ============================================================================
// Fact（信号型，无字段）
// ============================================================================

USTRUCT(BlueprintType)
struct SAGASTATS_API FLightningOnGroundSignal
{
	GENERATED_BODY()
};

// ============================================================================
// ConditionNode
// ============================================================================

UCLASS(BlueprintType, meta = (DisplayName = "LightningOnGround"))
class SAGASTATS_API UConditionNode_LightningOnGround : public UConditionNode_DPUBase
{
	GENERATED_BODY()
public:
	virtual UScriptStruct* GetConsumedFactType() const override { return FLightningOnGroundSignal::StaticStruct(); }
};

// ============================================================================
// Logic
// ============================================================================

UCLASS()
class SAGASTATS_API UDPULogic_LightningOnGround : public UDPULogicBase
{
	GENERATED_BODY()
public:
	virtual FInstancedStruct Execute_Implementation(const UDamageContext* DC) override;
};
