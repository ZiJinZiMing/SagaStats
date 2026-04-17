/***************************************************************************************************************
* Plugin:       SagaStats
* Author:       Jinming Zhang
* Description:  SagaStats offers modular damage process and meter systems to support adaptable status management
****************************************************************************************************************/

// Sekiro/DamagePipelineTestActor.h — 只狼 DamagePipeline 可视化运行时测试 Actor
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "DamagePipeline/DamagePipeline.h"
#include "DamagePipelineTestActor.generated.h"

/**
 * 将此 Actor 放置在测试关卡中。
 * BeginPlay 时构建 Sekiro MVP Pipeline（5 个 DamageRule：
 *   Mixup / Guard / Hurt / Collapse / CollapseJustGuard），
 * 运行 3 个测试场景（NormalHit / Guard / JustGuard），并在屏幕和日志中打印结果。
 */
UCLASS(HideDropDown)
class SAGASTATS_API ADamagePipelineTestActor : public AActor
{
	GENERATED_BODY()

public:
	ADamagePipelineTestActor();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DamageRule Test")
	float ScreenMessageDuration = 15.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DamageRule Test")
	bool bAutoRunOnBeginPlay = true;

	UFUNCTION(BlueprintCallable, Category = "DamageRule Test")
	void RunScenario(int32 ScenarioIndex);

	UFUNCTION(BlueprintCallable, Category = "DamageRule Test")
	void RunAllScenarios();

protected:
	virtual void BeginPlay() override;

private:
	UPROPERTY()
	TObjectPtr<UDamagePipeline> Pipeline;

	void BuildSekiroPipeline();
	void PrintToScreen(const FString& Message, FColor Color = FColor::White) const;
	void PrintScenarioResult(const FString& ScenarioName, UDamageContext* Context,
		const TArray<FRuleExecutionEntry>& Log) const;

	void RunScenario_NormalHit();
	void RunScenario_Guard();
	void RunScenario_JustGuard();
};
