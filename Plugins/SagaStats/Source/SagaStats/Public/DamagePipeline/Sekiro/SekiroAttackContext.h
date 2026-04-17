/***************************************************************************************************************
* Plugin:       SagaStats
* Author:       Jinming Zhang
* Description:  SagaStats offers modular damage process and meter systems to support adaptable status management
****************************************************************************************************************/

// SekiroAttackContext.h — 只狼测试场景的攻击上下文 USTRUCT
//
// FSekiroAttackContext 是 Game 侧（TestActor）在 Pipeline.Execute 之前通过
// UDamagePipelineResults::WriteEffect 预填到 DC 里的"攻击事件原始描述"。它不是任何
// Rule 的产出——无 Producer，永远以 "外部输入" 形式存在于 DC 里。
//
// 保留：被 DR_Mixup 消费（读 GuardLevel/DmgLevel 判定 Mixup Effect）。
#pragma once

#include "CoreMinimal.h"
#include "SekiroAttackContext.generated.h"

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
	bool bIsPlayer = false;
};
