// DPU_Collapse.h — 崩架势机制（Collapse + CollapseGuard，各自独立 Fact 类型）
#pragma once

#include "CoreMinimal.h"
#include "StructUtils/InstancedStruct.h"
#include "DPUFramework/DPULogicBase.h"
#include "DPUFramework/ConditionNode.h"
#include "DPUFramework/DamageContext.h"
#include "DPU_Collapse.generated.h"

// ============================================================================
// Fact — 每个 DPU 独立的 Fact 类型（DPU 和 Fact 一一对应）
// ============================================================================

USTRUCT(BlueprintType)
struct SAGASTATS_API FCollapseResult
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsCollapse = false;
};

USTRUCT(BlueprintType)
struct SAGASTATS_API FCollapseGuardResult
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsCollapse = false;
};

// ============================================================================
// ConditionNode — Collapse
// ============================================================================

UCLASS(BlueprintType, meta = (DisplayName = "Collapse"))
class SAGASTATS_API UConditionNode_Collapse : public UConditionNode_DPUBase
{
	GENERATED_BODY()
public:
	virtual UScriptStruct* GetConsumedFactType() const override { return FCollapseResult::StaticStruct(); }

	UFUNCTION() bool IsCollapse(const UDamageContext* DC) const;
};

// ============================================================================
// ConditionNode — CollapseGuard
// ============================================================================

UCLASS(BlueprintType, meta = (DisplayName = "CollapseGuard"))
class SAGASTATS_API UConditionNode_CollapseGuard : public UConditionNode_DPUBase
{
	GENERATED_BODY()
public:
	virtual UScriptStruct* GetConsumedFactType() const override { return FCollapseGuardResult::StaticStruct(); }

	UFUNCTION() bool IsCollapse(const UDamageContext* DC) const;
};

// ============================================================================
// Logic
// ============================================================================

UCLASS()
class SAGASTATS_API UDPULogic_Collapse : public UDPULogicBase
{
	GENERATED_BODY()
public:
	virtual FInstancedStruct Execute_Implementation(const UDamageContext* DC) override;
};

UCLASS()
class SAGASTATS_API UDPULogic_CollapseGuard : public UDPULogicBase
{
	GENERATED_BODY()
public:
	virtual FInstancedStruct Execute_Implementation(const UDamageContext* DC) override;
};
