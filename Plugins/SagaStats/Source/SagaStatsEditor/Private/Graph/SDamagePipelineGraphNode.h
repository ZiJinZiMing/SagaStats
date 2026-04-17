/***************************************************************************************************************
* Plugin:       SagaStats
* Author:       Claude(@JinmingZhang)
* Description:  SagaStats offers modular damage process and meter systems to support adaptable status management
****************************************************************************************************************/

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
 * - 标题区：N. RuleName（1-based 拓扑序号）
 * - 内容区：Condition ASCII 树（等宽字体 + 每行 EffectType 色块）+ Description（灰色小字）
 * - Pin 区：左输入 / 右输出（AutoWidth 撑开节点）
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

	/**
	 * 返回 Pin 区的 Desired Height —— 节点内"左输入/右输出"SHorizontalBox 的高度。
	 *
	 * 用途：让 Y 方向布局只避让 Pin 区（因为连线的 R1 约束只要求 pin Y 唯一），
	 * 表头区（Title + Condition + Description）可以在 Y 上和上一节点的下半区重叠。
	 * 配合 X 方向阶梯，表头不会真的互相覆盖（X 不同）；整张图垂直紧凑度大幅提升。
	 *
	 * 调用前需先 SlatePrepass 以保证 DesiredSize 为真实值。
	 */
	float GetPinAreaDesiredHeight() const;

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

	/** Pin 区的 HBox 句柄——SAssignNew 绑定，GetPinAreaDesiredHeight 查询其 DesiredSize.Y */
	TSharedPtr<class SHorizontalBox> PinAreaBox;
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
