// DPU_SuperArmor.h — 超级装甲信号（Fact + ConditionNode + Logic）
#pragma once

#include "CoreMinimal.h"
#include "StructUtils/InstancedStruct.h"
#include "DPUFramework/DPULogicBase.h"
#include "DPUFramework/DPUCondition.h"
#include "DPUFramework/DamageContext.h"
#include "DPU_SuperArmor.generated.h"

// ============================================================================
// Fact（信号型，无字段）
// ============================================================================

USTRUCT(BlueprintType)
struct SAGASTATS_API FSuperArmorSignal
{
	GENERATED_BODY()
};

// ============================================================================
// Condition
// ============================================================================

UCLASS(BlueprintType, meta = (DisplayName = "SuperArmor"))
class SAGASTATS_API UDPUCondition_SuperArmor : public UDPUCondition
{
	GENERATED_BODY()
public:
	virtual UScriptStruct* GetConsumedFactType() const override { return FSuperArmorSignal::StaticStruct(); }
	virtual bool Evaluate_Implementation(const UDamageContext* DC, const FInstancedStruct& ConsumedFact) const override { return ConsumedFact.IsValid(); }
};

// ============================================================================
// Logic
// ============================================================================

UCLASS()
class SAGASTATS_API UDPULogic_SuperArmor : public UDPULogicBase
{
	GENERATED_BODY()
public:
	virtual void Execute_Implementation(UDamageContext* DC, FInstancedStruct& OutFact) override;
	virtual UScriptStruct* GetProducesFactType() const override { return FSuperArmorSignal::StaticStruct(); }
};
