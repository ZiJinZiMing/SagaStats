// SDamagePipelineGraphNode.h — DamagePipeline 图表节点的自定义 Slate 外观
#pragma once

#include "CoreMinimal.h"
#include "SGraphNode.h"
#include "EdGraphUtilities.h"

class UDamagePipelineGraphNode;

/**
 * SDamagePipelineGraphNode
 *
 * DamageRule 图表节点的自定义可视化：
 * - 标题区：#N RuleName
 * - 内容区：Condition 文本 + Produces 类型名
 * - 浅蓝背景色
 */
class SDamagePipelineGraphNode : public SGraphNode
{
public:
	SLATE_BEGIN_ARGS(SDamagePipelineGraphNode) {}
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs, UDamagePipelineGraphNode* InNode);

	// SGraphNode overrides
	virtual void UpdateGraphNode() override;

private:
	/** 取得 "#N RuleName" 标题文本 */
	FText GetNodeTitle() const;

	/** 取得 "Cond: ..." 条件文本 */
	FText GetConditionText() const;

	/** 取得 "Produces: TypeName" 产出类型文本 */
	FText GetProducesText() const;

	/** 缓存的 UDamagePipelineGraphNode 指针 */
	UDamagePipelineGraphNode* PipelineGraphNode = nullptr;
};

/**
 * FDamagePipelineNodeFactory
 *
 * 当编辑器遇到 UDamagePipelineGraphNode 时，
 * 自动创建 SDamagePipelineGraphNode 作为其 Slate 表示。
 */
class FDamagePipelineNodeFactory : public FGraphPanelNodeFactory
{
public:
	virtual TSharedPtr<SGraphNode> CreateNode(UEdGraphNode* InNode) const override;
};
