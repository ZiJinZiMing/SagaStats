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
 * 布局方向为从左到右（left-to-right），按拓扑层级水平排列，层内垂直排列。
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
	 * - 自动布局：按拓扑层级从左到右，层内从上到下
	 */
	void RebuildGraph(UDamagePipeline* Pipeline);
};
