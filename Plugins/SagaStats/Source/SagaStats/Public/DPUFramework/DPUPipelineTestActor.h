// DPUPipelineTestActor.h — DPU Pipeline 可视化运行时测试 Actor
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "DPUFramework/PipelineAsset.h"
#include "DPUPipelineTestActor.generated.h"

/**
 * 将此 Actor 放置在测试关卡中。
 * BeginPlay 时构建 Sekiro MVP v4 Pipeline（13 个 DPU），
 * 运行多个测试场景，并在屏幕和日志中打印结果。
 */
UCLASS(BlueprintType, Blueprintable)
class SAGASTATS_API ADPUPipelineTestActor : public AActor
{
	GENERATED_BODY()

public:
	ADPUPipelineTestActor();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DPU Test")
	float ScreenMessageDuration = 15.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DPU Test")
	bool bAutoRunOnBeginPlay = true;

	UFUNCTION(BlueprintCallable, Category = "DPU Test")
	void RunScenario(int32 ScenarioIndex);

	UFUNCTION(BlueprintCallable, Category = "DPU Test")
	void RunAllScenarios();

protected:
	virtual void BeginPlay() override;

private:
	UPROPERTY()
	TObjectPtr<UPipelineAsset> Pipeline;

	void BuildSekiroPipeline();
	void PrintToScreen(const FString& Message, FColor Color = FColor::White) const;
	void PrintScenarioResult(const FString& ScenarioName, UDamageContext* DC,
		const TArray<FDPUExecutionEntry>& Log) const;

	void RunScenario_NormalHit();
	void RunScenario_Guard();
	void RunScenario_JustGuard();
	void RunScenario_LightningGround();
	void RunScenario_LightningInAir();
};
