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
 * 布局采用阶梯算法（Staircase）：节点按执行序从左到右排列，同时按累积 Pin 行数
 * 向下阶梯错位，保证每个 Pin 的 Y 全局唯一——水平段按构造不重合。
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
	 * - 调用阶梯布局引擎生成坐标
	 */
	void RebuildGraph(UDamagePipeline* Pipeline);
};
