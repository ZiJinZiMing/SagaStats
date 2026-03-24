// PipelineAsset.h — UPipelineAsset: 可序列化的 Pipeline 定义
#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "DPUFramework/DPUDefinition.h"
#include "PipelineAsset.generated.h"

/**
 * UPipelineAsset — 存储一组 DPU 定义的 Pipeline 定义。
 * 两种来源：编辑器构建（固定 Pipeline）或运行时组装（动态 Pipeline）。
 * 两者产生相同的资产类型 — 运行时引擎不做区分。
 */
UCLASS(BlueprintType)
class SAGASTATS_API UPipelineAsset : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName PipelineName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<TObjectPtr<UDPUDefinition>> DPUDefinitions;
};
