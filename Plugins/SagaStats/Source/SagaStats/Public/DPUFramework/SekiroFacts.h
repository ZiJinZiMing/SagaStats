// SekiroFacts.h — 只狼 MVP 的 Fact 结构体定义（纯数据）
// v4.6: 领域方法定义在 ConditionNode 子类上（ConditionNode.h），Fact 结构体只含数据字段。
#pragma once

#include "CoreMinimal.h"
#include "SekiroFacts.generated.h"

// ============================================================================
// 复杂 Fact
// ============================================================================

/** 猜拳结果 — DPU_Mixup 产出，Fact Key: "MixupResult" */
USTRUCT(BlueprintType)
struct SAGASTATS_API FMixupResult
{
	GENERATED_BODY()

	/** 是否触发了格挡（包含弹刀） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsGuard = false;

	/** 是否触发了弹刀（完美格挡） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsJustGuard = false;

	// 领域方法通过 FactMethodRegistry 注册：
	// IsGuard()        → bIsGuard
	// IsJustGuard()    → bIsJustGuard
	// IsGuardSuccess() → bIsGuard（格挡成功 = 格挡触发）
};

/** 格挡结果 — DPU_Guard 产出，Fact Key: "GuardResult" */
USTRUCT(BlueprintType)
struct SAGASTATS_API FGuardResult
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bGuardSuccess = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsJustGuard = false;

	// 领域方法：
	// IsGuardSuccess() → bGuardSuccess
	// IsJustGuard()    → bIsJustGuard
};

/** 受击结果 — DPU_Hurt 产出，Fact Key: "HurtResult" */
USTRUCT(BlueprintType)
struct SAGASTATS_API FHurtResult
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsHurt = false;

	// 领域方法：
	// IsHurt() → bIsHurt
};

/** 死亡结果 — DPU_Death 产出，Fact Key: "DeathResult" */
USTRUCT(BlueprintType)
struct SAGASTATS_API FDeathResult
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsDead = false;

	// 领域方法：
	// IsDead() → bIsDead
};

/** 崩架势结果 — DPU_Collapse / DPU_CollapseGuard 产出（互斥），Fact Key: "CollapseResult" */
USTRUCT(BlueprintType)
struct SAGASTATS_API FCollapseResult
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsCollapse = false;

	// 领域方法：
	// IsCollapse() → bIsCollapse
};

/** 中毒结果 — DPU_Poison 产出，Fact Key: "PoisonResult" */
USTRUCT(BlueprintType)
struct SAGASTATS_API FPoisonResult
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsPoisoned = false;

	// 领域方法：
	// IsPoisoned() → bIsPoisoned
};

/** 弹开结果 — DPU_AttackerBound 产出，Fact Key: "AttackerBoundResult" */
USTRUCT(BlueprintType)
struct SAGASTATS_API FAttackerBoundResult
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsBound = false;

	// 领域方法：
	// IsBound() → bIsBound
};

// ============================================================================
// 信号 Fact（空结构体，存在即 true）
// ============================================================================

/** 空中接雷信号 — DPU_LightningInAir 产出，Fact Key: "LightningInAirResult" */
USTRUCT(BlueprintType)
struct SAGASTATS_API FLightningInAirSignal
{
	GENERATED_BODY()
};

/** 地面雷击信号 — DPU_LightningOnGround 产出，Fact Key: "LightningOnGroundResult" */
USTRUCT(BlueprintType)
struct SAGASTATS_API FLightningOnGroundSignal
{
	GENERATED_BODY()
};

/** 强韧信号 — DPU_Toughness 产出，Fact Key: "ToughnessResult" */
USTRUCT(BlueprintType)
struct SAGASTATS_API FToughnessSignal
{
	GENERATED_BODY()
};

/** SuperArmor 信号 — DPU_SuperArmor 产出，Fact Key: "SuperArmorResult" */
USTRUCT(BlueprintType)
struct SAGASTATS_API FSuperArmorSignal
{
	GENERATED_BODY()
};

/** 弹刀崩架势信号 — DPU_CollapseJustGuard 产出，Fact Key: "CollapseJustGuardResult" */
USTRUCT(BlueprintType)
struct SAGASTATS_API FCollapseJustGuardSignal
{
	GENERATED_BODY()
};

// v4.6: 领域方法定义在 ConditionNode 子类上（ConditionNode.h），不再需要全局注册函数。
