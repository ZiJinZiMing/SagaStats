/***************************************************************************************************************
* Plugin:       SagaStats
* Author:       Jinming Zhang
* Description:  SagaStats offers modular damage process and meter systems to support adaptable status management
****************************************************************************************************************/

// DR_AttackContext.h — 只狼攻击上下文（Effect + Condition）
#pragma once

#include "CoreMinimal.h"
#include "StructUtils/InstancedStruct.h"
#include "DamagePipeline/DamageCondition.h"
#include "DamagePipeline/DamageContext.h"
#include "DR_AttackContext.generated.h"

// ============================================================================
// Effect
// ============================================================================

USTRUCT(BlueprintType)
struct SAGASTATS_API FSekiroAttackContext
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float DmgLevel = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float CurrentHP = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float GuardLevel = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bLightning = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsInAir = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsPlayer = false;
};

// ============================================================================
// Condition
// ============================================================================

UCLASS(BlueprintType, HideDropDown, meta = (DisplayName = "IsLightning"))
class SAGASTATS_API UDamageCondition_IsLightning : public UDamageCondition
{
	GENERATED_BODY()
public:
	virtual UScriptStruct* GetEffectType() const override { return FSekiroAttackContext::StaticStruct(); }
	virtual bool Evaluate_Implementation(const UDamageContext* Context, const FInstancedStruct& ConsumedEffect) const override;
};

UCLASS(BlueprintType, HideDropDown, meta = (DisplayName = "IsInAir"))
class SAGASTATS_API UDamageCondition_IsInAir : public UDamageCondition
{
	GENERATED_BODY()
public:
	virtual UScriptStruct* GetEffectType() const override { return FSekiroAttackContext::StaticStruct(); }
	virtual bool Evaluate_Implementation(const UDamageContext* Context, const FInstancedStruct& ConsumedEffect) const override;
};
