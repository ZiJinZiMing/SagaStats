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
	/** 取得 "1. RuleName" 标题文本（拓扑序号 1-based + Rule 名）*/
	FText GetNodeTitle() const;

	/** Rule->Description 原文；空时返回空字符串 */
	FText GetDescriptionText() const;

	/** Description 区可见性：空时 Collapsed（节点自动缩紧），非空时 Visible */
	EVisibility GetDescriptionVisibility() const;

	/** UpdateGraphNode 中按 Condition 树行动态构造条件行（文本 + 可选色块） */
	void PopulateConditionBox();

	/** 缓存的 UDamagePipelineGraphNode 指针 */
	UDamagePipelineGraphNode* PipelineGraphNode = nullptr;

	/** Condition 区的 VBox 句柄——SAssignNew 绑定，PopulateConditionBox 往里追加行 */
	TSharedPtr<class SVerticalBox> ConditionBox;
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
