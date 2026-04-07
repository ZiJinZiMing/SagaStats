// DPU_Collapse.h — 崩架势机制（Collapse + CollapseGuard，各自独立 Fact 类型）
#pragma once

#include "CoreMinimal.h"
#include "StructUtils/InstancedStruct.h"
#include "DPUFramework/DPULogicBase.h"
#include "DPUFramework/DPUCondition.h"
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
// Condition — Collapse
// ============================================================================

UCLASS(BlueprintType, meta = (DisplayName = "CollapseIsCollapse"))
class SAGASTATS_API UDPUCondition_CollapseIsCollapse : public UDPUCondition
{
	GENERATED_BODY()
public:
	virtual UScriptStruct* GetConsumedFactType() const override { return FCollapseResult::StaticStruct(); }
	virtual bool Evaluate_Implementation(const UDamageContext* DC, const FInstancedStruct& ConsumedFact) const override;
};

// ============================================================================
// Condition — CollapseGuard
// ============================================================================

UCLASS(BlueprintType, meta = (DisplayName = "CollapseGuardIsCollapse"))
class SAGASTATS_API UDPUCondition_CollapseGuardIsCollapse : public UDPUCondition
{
	GENERATED_BODY()
public:
	virtual UScriptStruct* GetConsumedFactType() const override { return FCollapseGuardResult::StaticStruct(); }
	virtual bool Evaluate_Implementation(const UDamageContext* DC, const FInstancedStruct& ConsumedFact) const override;
};

// ============================================================================
// Logic
// ============================================================================

UCLASS()
class SAGASTATS_API UDPULogic_Collapse : public UDPULogicBase
{
	GENERATED_BODY()
public:
	virtual void Execute_Implementation(UDamageContext* DC, FInstancedStruct& OutFact) override;
	virtual UScriptStruct* GetProducesFactType() const override { return FCollapseResult::StaticStruct(); }
};

UCLASS()
class SAGASTATS_API UDPULogic_CollapseGuard : public UDPULogicBase
{
	GENERATED_BODY()
public:
	virtual void Execute_Implementation(UDamageContext* DC, FInstancedStruct& OutFact) override;
	virtual UScriptStruct* GetProducesFactType() const override { return FCollapseGuardResult::StaticStruct(); }
};
