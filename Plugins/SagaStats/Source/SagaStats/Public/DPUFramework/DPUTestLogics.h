// DPUTestLogics.h — 只狼 MVP v4 验证用的测试 DPU 逻辑子类
// 仅用于测试，不作为正式游戏逻辑。
#pragma once

#include "CoreMinimal.h"
#include "DPUFramework/DPULogicBase.h"
#include "DPUFramework/DamageContext.h"
#include "DPUTestLogics.generated.h"

// DPU_Mixup: 博弈判定 — 读取 guard_level，写入 Guard
UCLASS()
class SAGASTATS_API UDPULogic_TestMixup : public UDPULogicBase
{
	GENERATED_BODY()
public:
	virtual void Execute(UDamageContext* DC) override
	{
		float GuardLevel = DC->GetFloat(FName("guard_level"));
		DC->SetBool(FName("Guard"), GuardLevel > 0.f);
	}
};

// DPU_Guard: 格挡判定 — 读取 Guard, DmgLevel，写入 IsJustGuard, GuardSuccess
UCLASS()
class SAGASTATS_API UDPULogic_TestGuard : public UDPULogicBase
{
	GENERATED_BODY()
public:
	virtual void Execute(UDamageContext* DC) override
	{
		float GuardLevel = DC->GetFloat(FName("guard_level"));
		float DmgLevel = DC->GetFloat(FName("DmgLevel"));
		DC->SetBool(FName("GuardSuccess"), GuardLevel >= DmgLevel);
		DC->SetBool(FName("IsJustGuard"), GuardLevel > DmgLevel);
	}
};

// DPU_Death: HP结算 — 写入 IsDeath
UCLASS()
class SAGASTATS_API UDPULogic_TestDeath : public UDPULogicBase
{
	GENERATED_BODY()
public:
	virtual void Execute(UDamageContext* DC) override
	{
		float HP = DC->GetFloat(FName("CurrentHP"));
		float Damage = DC->GetFloat(FName("DmgLevel"));
		DC->SetBool(FName("IsDeath"), HP <= Damage);
	}
};

// DPU_Hurt: 受击表现 — 写入 HurtExecuted 标记
UCLASS()
class SAGASTATS_API UDPULogic_TestHurt : public UDPULogicBase
{
	GENERATED_BODY()
public:
	virtual void Execute(UDamageContext* DC) override
	{
		DC->SetBool(FName("HurtExecuted"), true);
	}
};

// DPU_LightningInAir: 空中接雷 — 写入 IsLightningInAir
UCLASS()
class SAGASTATS_API UDPULogic_TestLightningInAir : public UDPULogicBase
{
	GENERATED_BODY()
public:
	virtual void Execute(UDamageContext* DC) override
	{
		DC->SetBool(FName("IsLightningInAir"), true);
	}
};

// DPU_Collapse: 崩架势 — 写入 IsCollapse
UCLASS()
class SAGASTATS_API UDPULogic_TestCollapse : public UDPULogicBase
{
	GENERATED_BODY()
public:
	virtual void Execute(UDamageContext* DC) override
	{
		DC->SetBool(FName("IsCollapse"), true);
	}
};

// DPU_CollapseGuard: 破防 — 写入 IsCollapse
UCLASS()
class SAGASTATS_API UDPULogic_TestCollapseGuard : public UDPULogicBase
{
	GENERATED_BODY()
public:
	virtual void Execute(UDamageContext* DC) override
	{
		DC->SetBool(FName("IsCollapse"), true);
	}
};

// DPU_CollapseJustGuard: 弹刀崩架势 — 标记
UCLASS()
class SAGASTATS_API UDPULogic_TestCollapseJustGuard : public UDPULogicBase
{
	GENERATED_BODY()
public:
	virtual void Execute(UDamageContext* DC) override
	{
		DC->SetBool(FName("CollapseJustGuardExecuted"), true);
	}
};

// DPU_AttackerBound: 弹开攻击方 — 写入 IsBound
UCLASS()
class SAGASTATS_API UDPULogic_TestAttackerBound : public UDPULogicBase
{
	GENERATED_BODY()
public:
	virtual void Execute(UDamageContext* DC) override
	{
		DC->SetBool(FName("IsBound"), true);
	}
};

// DPU_Poison: 中毒 — 写入 IsPoisoned
UCLASS()
class SAGASTATS_API UDPULogic_TestPoison : public UDPULogicBase
{
	GENERATED_BODY()
public:
	virtual void Execute(UDamageContext* DC) override
	{
		DC->SetBool(FName("IsPoisoned"), true);
	}
};

// DPU_Toughness: 强韧 — 写入 IsTough
UCLASS()
class SAGASTATS_API UDPULogic_TestToughness : public UDPULogicBase
{
	GENERATED_BODY()
public:
	virtual void Execute(UDamageContext* DC) override
	{
		DC->SetBool(FName("IsTough"), true);
	}
};

// DPU_SuperArmor — 写入 IsInSuperArmor
UCLASS()
class SAGASTATS_API UDPULogic_TestSuperArmor : public UDPULogicBase
{
	GENERATED_BODY()
public:
	virtual void Execute(UDamageContext* DC) override
	{
		DC->SetBool(FName("IsInSuperArmor"), true);
	}
};

// DPU_LightningOnGround — 标记
UCLASS()
class SAGASTATS_API UDPULogic_TestLightningOnGround : public UDPULogicBase
{
	GENERATED_BODY()
public:
	virtual void Execute(UDamageContext* DC) override
	{
		DC->SetBool(FName("LightningOnGroundExecuted"), true);
	}
};
