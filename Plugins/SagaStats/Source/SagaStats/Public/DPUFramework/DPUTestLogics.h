// DPUTestLogics.h — 只狼 MVP v4 验证用的测试 DPU 逻辑子类
// v4.5: Execute 返回 FInstancedStruct（Fact），不再写入 DC。PipelineAsset 负责以 DPU 名写入。
#pragma once

#include "CoreMinimal.h"
#include "DPUFramework/DPULogicBase.h"
#include "DPUFramework/DamageContext.h"
#include "DPUFramework/SekiroFacts.h"
#include "DPUTestLogics.generated.h"

// ============================================================================
// DPU_Mixup: 博弈判定
// 读取事件上下文中的 guard_level，返回 FMixupResult
// ============================================================================
UCLASS()
class SAGASTATS_API UDPULogic_TestMixup : public UDPULogicBase
{
	GENERATED_BODY()
public:
	virtual FInstancedStruct Execute_Implementation(const UDamageContext* DC) override
	{
		float GuardLevel = DC->GetFloat(FName("guard_level"));
		float DmgLevel = DC->GetFloat(FName("DmgLevel"));
		FMixupResult Result;
		Result.bIsGuard = GuardLevel > 0.f;
		Result.bIsJustGuard = GuardLevel > DmgLevel;
		return FInstancedStruct::Make<FMixupResult>(Result);
	}
};

// ============================================================================
// DPU_Guard: 格挡判定
// 读取 Mixup 的 Fact，返回 FGuardResult
// ============================================================================
UCLASS()
class SAGASTATS_API UDPULogic_TestGuard : public UDPULogicBase
{
	GENERATED_BODY()
public:
	virtual FInstancedStruct Execute_Implementation(const UDamageContext* DC) override
	{
		const FMixupResult* Mixup = DC->GetFact<FMixupResult>(FName("Mixup"));
		FGuardResult Result;
		if (Mixup)
		{
			Result.bGuardSuccess = Mixup->bIsGuard;
			Result.bIsJustGuard = Mixup->bIsJustGuard;
		}
		return FInstancedStruct::Make<FGuardResult>(Result);
	}
};

// ============================================================================
// DPU_Death: HP结算 → 死亡判定
// ============================================================================
UCLASS()
class SAGASTATS_API UDPULogic_TestDeath : public UDPULogicBase
{
	GENERATED_BODY()
public:
	virtual FInstancedStruct Execute_Implementation(const UDamageContext* DC) override
	{
		float HP = DC->GetFloat(FName("CurrentHP"));
		float Damage = DC->GetFloat(FName("DmgLevel"));
		FDeathResult Result;
		Result.bIsDead = HP <= Damage;
		return FInstancedStruct::Make<FDeathResult>(Result);
	}
};

// ============================================================================
// DPU_Hurt: 受击表现
// ============================================================================
UCLASS()
class SAGASTATS_API UDPULogic_TestHurt : public UDPULogicBase
{
	GENERATED_BODY()
public:
	virtual FInstancedStruct Execute_Implementation(const UDamageContext* DC) override
	{
		FHurtResult Result;
		Result.bIsHurt = true;
		return FInstancedStruct::Make<FHurtResult>(Result);
	}
};

// ============================================================================
// DPU_LightningInAir: 空中接雷（信号 Fact）
// ============================================================================
UCLASS()
class SAGASTATS_API UDPULogic_TestLightningInAir : public UDPULogicBase
{
	GENERATED_BODY()
public:
	virtual FInstancedStruct Execute_Implementation(const UDamageContext* DC) override
	{
		return FInstancedStruct::Make<FLightningInAirSignal>(FLightningInAirSignal{});
	}
};

// ============================================================================
// DPU_Collapse: 崩架势
// ============================================================================
UCLASS()
class SAGASTATS_API UDPULogic_TestCollapse : public UDPULogicBase
{
	GENERATED_BODY()
public:
	virtual FInstancedStruct Execute_Implementation(const UDamageContext* DC) override
	{
		FCollapseResult Result;
		Result.bIsCollapse = true;
		return FInstancedStruct::Make<FCollapseResult>(Result);
	}
};

// ============================================================================
// DPU_CollapseGuard: 破防
// ============================================================================
UCLASS()
class SAGASTATS_API UDPULogic_TestCollapseGuard : public UDPULogicBase
{
	GENERATED_BODY()
public:
	virtual FInstancedStruct Execute_Implementation(const UDamageContext* DC) override
	{
		FCollapseResult Result;
		Result.bIsCollapse = true;
		return FInstancedStruct::Make<FCollapseResult>(Result);
	}
};

// ============================================================================
// DPU_CollapseJustGuard: 弹刀崩架势（信号 Fact）
// ============================================================================
UCLASS()
class SAGASTATS_API UDPULogic_TestCollapseJustGuard : public UDPULogicBase
{
	GENERATED_BODY()
public:
	virtual FInstancedStruct Execute_Implementation(const UDamageContext* DC) override
	{
		return FInstancedStruct::Make<FCollapseJustGuardSignal>(FCollapseJustGuardSignal{});
	}
};

// ============================================================================
// DPU_AttackerBound: 弹开攻击方
// ============================================================================
UCLASS()
class SAGASTATS_API UDPULogic_TestAttackerBound : public UDPULogicBase
{
	GENERATED_BODY()
public:
	virtual FInstancedStruct Execute_Implementation(const UDamageContext* DC) override
	{
		FAttackerBoundResult Result;
		Result.bIsBound = true;
		return FInstancedStruct::Make<FAttackerBoundResult>(Result);
	}
};

// ============================================================================
// DPU_Poison: 中毒
// ============================================================================
UCLASS()
class SAGASTATS_API UDPULogic_TestPoison : public UDPULogicBase
{
	GENERATED_BODY()
public:
	virtual FInstancedStruct Execute_Implementation(const UDamageContext* DC) override
	{
		FPoisonResult Result;
		Result.bIsPoisoned = true;
		return FInstancedStruct::Make<FPoisonResult>(Result);
	}
};

// ============================================================================
// DPU_Toughness: 强韧（信号 Fact）
// ============================================================================
UCLASS()
class SAGASTATS_API UDPULogic_TestToughness : public UDPULogicBase
{
	GENERATED_BODY()
public:
	virtual FInstancedStruct Execute_Implementation(const UDamageContext* DC) override
	{
		return FInstancedStruct::Make<FToughnessSignal>(FToughnessSignal{});
	}
};

// ============================================================================
// DPU_SuperArmor（信号 Fact）
// ============================================================================
UCLASS()
class SAGASTATS_API UDPULogic_TestSuperArmor : public UDPULogicBase
{
	GENERATED_BODY()
public:
	virtual FInstancedStruct Execute_Implementation(const UDamageContext* DC) override
	{
		return FInstancedStruct::Make<FSuperArmorSignal>(FSuperArmorSignal{});
	}
};

// ============================================================================
// DPU_LightningOnGround（信号 Fact）
// ============================================================================
UCLASS()
class SAGASTATS_API UDPULogic_TestLightningOnGround : public UDPULogicBase
{
	GENERATED_BODY()
public:
	virtual FInstancedStruct Execute_Implementation(const UDamageContext* DC) override
	{
		return FInstancedStruct::Make<FLightningOnGroundSignal>(FLightningOnGroundSignal{});
	}
};
