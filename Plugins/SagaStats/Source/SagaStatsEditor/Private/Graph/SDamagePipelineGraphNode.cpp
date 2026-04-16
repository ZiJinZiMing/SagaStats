/***************************************************************************************************************
* Plugin:       SagaStats
* Author:       Claude(@JinmingZhang)
* Description:  SagaStats offers modular damage process and meter systems to support adaptable status management
****************************************************************************************************************/

// SDamagePipelineGraphNode.cpp
#include "Graph/SDamagePipelineGraphNode.h"
#include "Graph/DamagePipelineGraphNode.h"
#include "Graph/DamagePipelineGraphSchema.h"
#include "DamagePipeline/DamageRule.h"
#include "DamagePipeline/DamagePredicate.h"
#include "SGraphNode.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/SNullWidget.h"
#include "Widgets/Colors/SColorBlock.h"
#include "Styling/AppStyle.h"
#include "Styling/CoreStyle.h"

#define LOCTEXT_NAMESPACE "SDamagePipelineGraphNode"

// ---------------------------------------------------------------------
// SDamagePipelineGraphNode
// ---------------------------------------------------------------------

void SDamagePipelineGraphNode::Construct(const FArguments& InArgs, UDamagePipelineGraphNode* InNode)
{
	PipelineGraphNode = InNode;
	GraphNode = InNode;
	SetCursor(EMouseCursor::Default);
	UpdateGraphNode();
}

void SDamagePipelineGraphNode::UpdateGraphNode()
{
	InputPins.Empty();
	OutputPins.Empty();
	RightNodeBox.Reset();
	LeftNodeBox.Reset();

	this->ContentScale.Bind(this, &SGraphNode::GetContentScale);

	this->GetOrAddSlot(ENodeZone::Center)
		.HAlign(HAlign_Fill)
		.VAlign(VAlign_Center)
		[
			SNew(SBorder)
			.BorderImage(FAppStyle::GetBrush("Graph.Node.Body"))
			.BorderBackgroundColor(FLinearColor(0.83f, 0.91f, 0.99f))
			.Padding(0.f)
			[
				SNew(SVerticalBox)

				// --- Title area ---
				+ SVerticalBox::Slot()
				.AutoHeight()
				[
					SNew(SBorder)
					.BorderImage(FAppStyle::GetBrush("Graph.Node.TitleGloss"))
					.Padding(FMargin(10.f, 4.f))
					[
						SNew(STextBlock)
						.Text(this, &SDamagePipelineGraphNode::GetNodeTitle)
						.TextStyle(FAppStyle::Get(), "Graph.Node.NodeTitle")
					]
				]

				// --- Body: Condition rows + Description ---
				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(4.f)
				[
					SNew(SVerticalBox)

					// Condition 树：每行一个 SHorizontalBox（文本 + 可选色块），
					// 在 UpdateGraphNode 中静态构造（详见下方 CondBox 填充代码）
					+ SVerticalBox::Slot()
					.AutoHeight()
					[
						SAssignNew(ConditionBox, SVerticalBox)
					]

					// Description（空值自动缩紧；\n 硬换行 + AutoWrapText 软换行双重策略）
					+ SVerticalBox::Slot()
					.AutoHeight()
					.Padding(FMargin(0.f, 2.f, 0.f, 0.f))
					[
						SNew(STextBlock)
						.Visibility(this, &SDamagePipelineGraphNode::GetDescriptionVisibility)
						.Text(this, &SDamagePipelineGraphNode::GetDescriptionText)
						.ColorAndOpacity(FLinearColor(0.6f, 0.6f, 0.6f))
						.Font(FCoreStyle::GetDefaultFontStyle("Regular", 8))
						.AutoWrapText(true)
						.WrapTextAt(300.f)
					]
				]

				// --- Pin area ---
				// 左右两列都用 AutoWidth，中间 FillWidth spacer 拉开。
				// 这样 pin 的真实宽度会通过 desired size 向上传，撑开整个节点。
				+ SVerticalBox::Slot()
				.AutoHeight()
				[
					SNew(SHorizontalBox)

					+ SHorizontalBox::Slot()
					.AutoWidth()
					[
						SAssignNew(LeftNodeBox, SVerticalBox)
					]

					+ SHorizontalBox::Slot()
					.FillWidth(1.f)
					[
						SNullWidget::NullWidget
					]

					+ SHorizontalBox::Slot()
					.AutoWidth()
					.HAlign(HAlign_Right)
					[
						SAssignNew(RightNodeBox, SVerticalBox)
					]
				]
			]
		];

	PopulateConditionBox();

	CreatePinWidgets();
}

void SDamagePipelineGraphNode::PopulateConditionBox()
{
	if (!ConditionBox.IsValid())
	{
		return;
	}

	ConditionBox->ClearChildren();

	if (!PipelineGraphNode || !PipelineGraphNode->Rule.IsValid())
	{
		return;
	}

	UDamageRule* Rule = PipelineGraphNode->Rule.Get();
	const FLinearColor TextColor(0.7f, 0.7f, 0.2f);
	const FSlateFontInfo MonoFont = FCoreStyle::GetDefaultFontStyle("Mono", 9);

	if (!Rule->Condition)
	{
		// 无 Condition：单行 "(always)"
		ConditionBox->AddSlot()
			.AutoHeight()
			[
				SNew(STextBlock)
				.Text(LOCTEXT("CondAlways", "(always)"))
				.ColorAndOpacity(TextColor)
				.Font(MonoFont)
			];
		return;
	}

	// 递归收集 ASCII 树每一行
	TArray<FConditionDisplayLine> Lines;
	Rule->Condition->CollectDisplayLines(TEXT(""), TEXT(""), Lines);

	for (const FConditionDisplayLine& Line : Lines)
	{
		const bool bHasEffect = (Line.EffectType != nullptr);
		const FLinearColor SwatchColor = bHasEffect
			? UDamagePipelineGraphSchema::GetColorForEffectType(Line.EffectType)
			: FLinearColor::Transparent;

		ConditionBox->AddSlot()
			.AutoHeight()
			[
				SNew(SHorizontalBox)

				// 行文本（整行同色 mono 黄）
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.VAlign(VAlign_Center)
				[
					SNew(STextBlock)
					.Text(FText::FromString(Line.Text))
					.ColorAndOpacity(TextColor)
					.Font(MonoFont)
				]

				// 色块（只在 leaf 行可见；结构行 Collapsed 不占位）
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.VAlign(VAlign_Center)
				.Padding(FMargin(6.f, 0.f, 0.f, 0.f))
				[
					SNew(SColorBlock)
					.Visibility(bHasEffect ? EVisibility::Visible : EVisibility::Collapsed)
					.Color(SwatchColor)
					.Size(FVector2D(10.f, 10.f))
					.AlphaDisplayMode(EColorBlockAlphaDisplayMode::Ignore)
				]
			];
	}
}

FText SDamagePipelineGraphNode::GetNodeTitle() const
{
	if (!PipelineGraphNode || !PipelineGraphNode->Rule.IsValid())
	{
		return LOCTEXT("InvalidNode", "(Invalid)");
	}

	const FString RuleName = PipelineGraphNode->Rule->GetName();
	const int32 DisplayIndex = PipelineGraphNode->SortIndex + 1;
	return FText::FromString(FString::Printf(TEXT("%d. %s"), DisplayIndex, *RuleName));
}

FText SDamagePipelineGraphNode::GetDescriptionText() const
{
	if (PipelineGraphNode && PipelineGraphNode->Rule.IsValid())
	{
		return PipelineGraphNode->Rule->Description;
	}
	return FText::GetEmpty();
}

EVisibility SDamagePipelineGraphNode::GetDescriptionVisibility() const
{
	if (PipelineGraphNode && PipelineGraphNode->Rule.IsValid()
		&& !PipelineGraphNode->Rule->Description.IsEmpty())
	{
		return EVisibility::Visible;
	}
	return EVisibility::Collapsed;
}

// ---------------------------------------------------------------------
// FDamagePipelineNodeFactory
// ---------------------------------------------------------------------

TSharedPtr<SGraphNode> FDamagePipelineNodeFactory::CreateNode(UEdGraphNode* InNode) const
{
	if (UDamagePipelineGraphNode* DPNode = Cast<UDamagePipelineGraphNode>(InNode))
	{
		TSharedPtr<SDamagePipelineGraphNode> SNode = SNew(SDamagePipelineGraphNode, DPNode);
		return SNode;
	}
	return nullptr;
}

#undef LOCTEXT_NAMESPACE
