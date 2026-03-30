// DPUTestLogics.h — 只狼 MVP v4 验证用的测试 DPU 逻辑子类
// v4.4：使用真正的 Fact 结构体（SetFact<T>/GetFact<T>），而非散装标量。
#pragma once

#include "CoreMinimal.h"
#include "DPUFramework/DPULogicBase.h"
#include "DPUFramework/DamageContext.h"
#include "DPUFramework/SekiroFacts.h"
#include "DPUTestLogics.generated.h"

// ============================================================================
// Fact Key 常量（避免散装字符串）
// ============================================================================
namespace SekiroFactKeys
{
	const FName MixupResult      = FName("MixupResult");
	const FName GuardResult      = FName("GuardResult");
	const FName HurtResult       = FName("HurtResult");
	const FName DeathResult      = FName("DeathResult");
	const FName CollapseResult   = FName("CollapseResult");
	const FName PoisonResult     = FName("PoisonResult");
	const FName AttackerBoundResult = FName("AttackerBoundResult");
	const FName LightningInAirResult  = FName("LightningInAirResult");
	const FName LightningOnGroundResult = FName("LightningOnGroundResult");
	const FName ToughnessResult  = FName("ToughnessResult");
	const FName SuperArmorResult = FName("SuperArmorResult");
	const FName CollapseJustGuardResult = FName("CollapseJustGuardResult");
}

// ============================================================================
// DPU_Mixup: 博弈判定
// 读取事件上下文中的 guard_level，产出 FMixupResult
// ============================================================================
UCLASS()
class SAGASTATS_API UDPULogic_TestMixup : public UDPULogicBase
{
	GENERATED_BODY()
public:
	virtual void Execute_Implementation(UDamageContext* DC) override
	{
		float GuardLevel = DC->GetFloat(FName("guard_level"));
		FMixupResult Result;
		Result.bIsGuard = GuardLevel > 0.f;
		Result.bIsJustGuard = GuardLevel > DC->GetFloat(FName("DmgLevel"));
		DC->SetFact<FMixupResult>(SekiroFactKeys::MixupResult, Result);
	}
};

// ============================================================================
// DPU_Guard: 格挡判定
// 读取 MixupResult，产出 FGuardResult
// ============================================================================
UCLASS()
class SAGASTATS_API UDPULogic_TestGuard : public UDPULogicBase
{
	GENERATED_BODY()
public:
	virtual void Execute_Implementation(UDamageContext* DC) override
	{
		const FMixupResult* Mixup = DC->GetFact<FMixupResult>(SekiroFactKeys::MixupResult);
		FGuardResult Result;
		if (Mixup)
		{
			Result.bGuardSuccess = Mixup->bIsGuard;
			Result.bIsJustGuard = Mixup->bIsJustGuard;
		}
		DC->SetFact<FGuardResult>(SekiroFactKeys::GuardResult, Result);
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
	virtual void Execute_Implementation(UDamageContext* DC) override
	{
		float HP = DC->GetFloat(FName("CurrentHP"));
		float Damage = DC->GetFloat(FName("DmgLevel"));
		FDeathResult Result;
		Result.bIsDead = HP <= Damage;
		DC->SetFact<FDeathResult>(SekiroFactKeys::DeathResult, Result);
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
	virtual void Execute_Implementation(UDamageContext* DC) override
	{
		FHurtResult Result;
		Result.bIsHurt = true;
		DC->SetFact<FHurtResult>(SekiroFactKeys::HurtResult, Result);
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
	virtual void Execute_Implementation(UDamageContext* DC) override
	{
		DC->SetFact<FLightningInAirSignal>(SekiroFactKeys::LightningInAirResult, FLightningInAirSignal{});
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
	virtual void Execute_Implementation(UDamageContext* DC) override
	{
		FCollapseResult Result;
		Result.bIsCollapse = true;
		DC->SetFact<FCollapseResult>(SekiroFactKeys::CollapseResult, Result);
	}
};

// ============================================================================
// DPU_CollapseGuard: 破防（produces 同一个 CollapseResult，与 DPU_Collapse 互斥）
// ============================================================================
UCLASS()
class SAGASTATS_API UDPULogic_TestCollapseGuard : public UDPULogicBase
{
	GENERATED_BODY()
public:
	virtual void Execute_Implementation(UDamageContext* DC) override
	{
		FCollapseResult Result;
		Result.bIsCollapse = true;
		DC->SetFact<FCollapseResult>(SekiroFactKeys::CollapseResult, Result);
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
	virtual void Execute_Implementation(UDamageContext* DC) override
	{
		DC->SetFact<FCollapseJustGuardSignal>(SekiroFactKeys::CollapseJustGuardResult, FCollapseJustGuardSignal{});
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
	virtual void Execute_Implementation(UDamageContext* DC) override
	{
		FAttackerBoundResult Result;
		Result.bIsBound = true;
		DC->SetFact<FAttackerBoundResult>(SekiroFactKeys::AttackerBoundResult, Result);
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
	virtual void Execute_Implementation(UDamageContext* DC) override
	{
		FPoisonResult Result;
		Result.bIsPoisoned = true;
		DC->SetFact<FPoisonResult>(SekiroFactKeys::PoisonResult, Result);
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
	virtual void Execute_Implementation(UDamageContext* DC) override
	{
		DC->SetFact<FToughnessSignal>(SekiroFactKeys::ToughnessResult, FToughnessSignal{});
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
	virtual void Execute_Implementation(UDamageContext* DC) override
	{
		DC->SetFact<FSuperArmorSignal>(SekiroFactKeys::SuperArmorResult, FSuperArmorSignal{});
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
	virtual void Execute_Implementation(UDamageContext* DC) override
	{
		DC->SetFact<FLightningOnGroundSignal>(SekiroFactKeys::LightningOnGroundResult, FLightningOnGroundSignal{});
	}
};
