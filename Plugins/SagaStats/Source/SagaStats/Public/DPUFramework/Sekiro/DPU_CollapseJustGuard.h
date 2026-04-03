// DPU_CollapseJustGuard.h — 崩溃弹反信号（Fact + ConditionNode + Logic）
#pragma once

#include "CoreMinimal.h"
#include "StructUtils/InstancedStruct.h"
#include "DPUFramework/DPULogicBase.h"
#include "DPUFramework/ConditionNode.h"
#include "DPUFramework/DamageContext.h"
#include "DPU_CollapseJustGuard.generated.h"

// ============================================================================
// Fact（信号型，无字段）
// ============================================================================

USTRUCT(BlueprintType)
struct SAGASTATS_API FCollapseJustGuardSignal
{
	GENERATED_BODY()
};

// ============================================================================
// ConditionNode
// ============================================================================

UCLASS(BlueprintType, meta = (DisplayName = "CollapseJustGuard"))
class SAGASTATS_API UConditionNode_CollapseJustGuard : public UConditionNode_DPUBase
{
	GENERATED_BODY()
public:
	virtual UScriptStruct* GetConsumedFactType() const override { return FCollapseJustGuardSignal::StaticStruct(); }
};

// ============================================================================
// Logic
// ============================================================================

UCLASS()
class SAGASTATS_API UDPULogic_CollapseJustGuard : public UDPULogicBase
{
	GENERATED_BODY()
public:
	virtual FInstancedStruct Execute_Implementation(const UDamageContext* DC) override;
};
