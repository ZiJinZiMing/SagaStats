/***************************************************************************************************************
* Plugin:       SagaStats
* Author:       Claude(@JinmingZhang)
* Description:  SagaStats offers modular damage process and meter systems to support adaptable status management
****************************************************************************************************************/

// DamagePipelineGraph.h — DamagePipeline 的只读 DAG 可视化图表
#pragma once

#include "CoreMinimal.h"
#include "EdGraph/EdGraph.h"
#include "DamagePipelineGraph.generated.h"

class UDamagePipeline;

/**
 * UDamagePipelineGraph — DamagePipeline 的只读图表。
 *
 * 以 DamageRule 为节点、EffectType 产销关系为边，展示管线的 DAG 拓扑结构。
 * RebuildGraph 负责创建节点和连接 Pin（纯数据层职责），节点位置置 (0,0)。
 * 真实布局由 FDamagePipelineAssetEditor::ApplyRealSizeLayoutCorrection
 * 在 Slate widget 就绪后读取真实尺寸完成。
 */
UCLASS()
class UDamagePipelineGraph : public UEdGraph
{
	GENERATED_BODY()

public:
	/**
	 * 根据 Pipeline 重建整张图表。
	 * - 清除所有现有节点
	 * - 为每个 DamageRule 创建 UDamagePipelineGraphNode
	 * - 基于 EffectType 匹配（ProducesEffectType → ConsumedEffectType）创建 Pin 连接
	 * - 节点位置留空 (0,0)，真实布局由 AssetEditor Ticker 完成
	 */
	void RebuildGraph(UDamagePipeline* Pipeline);
};
