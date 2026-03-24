// DPUPipelineTestActor.h — DPU Pipeline 可视化运行时测试 Actor
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "DPUFramework/DPUDefinition.h"
#include "DPUFramework/PipelineManager.h"
#include "DPUPipelineTestActor.generated.h"

/**
 * 将此 Actor 放置在测试关卡中。
 * BeginPlay 时构建 Sekiro MVP v4 Pipeline（13 个 DPU），
 * 运行多个测试场景，并在屏幕和日志中打印结果。
 *
 * 运行时按 1~5 键重新运行单个场景。
 */
UCLASS(BlueprintType, Blueprintable)
class SAGASTATS_API ADPUPipelineTestActor : public AActor
{
	GENERATED_BODY()

public:
	ADPUPipelineTestActor();

	/** 屏幕消息保持可见的时长（秒） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DPU Test")
	float ScreenMessageDuration = 15.f;

	/** 如果为 true，在 BeginPlay 时自动运行所有场景 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DPU Test")
	bool bAutoRunOnBeginPlay = true;

	/** 按索引运行指定场景 (1-5) */
	UFUNCTION(BlueprintCallable, Category = "DPU Test")
	void RunScenario(int32 ScenarioIndex);

	/** 运行全部 5 个场景 */
	UFUNCTION(BlueprintCallable, Category = "DPU Test")
	void RunAllScenarios();

protected:
	virtual void BeginPlay() override;

private:
	UPROPERTY()
	TObjectPtr<UPipelineManager> PipelineManager;

	UPROPERTY()
	TArray<TObjectPtr<UDPUDefinition>> SekiroDPUs;

	void BuildSekiroPipeline();
	void PrintToScreen(const FString& Message, FColor Color = FColor::White) const;
	void PrintScenarioResult(const FString& ScenarioName, UDamageContext* DC,
		const TArray<FDPUExecutionEntry>& Log) const;

	// 场景执行方法
	void RunScenario_NormalHit();
	void RunScenario_Guard();
	void RunScenario_JustGuard();
	void RunScenario_LightningGround();
	void RunScenario_LightningInAir();
};
