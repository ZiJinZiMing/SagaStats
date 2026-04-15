// SDamagePipelineGraphNode.cpp
#include "Graph/SDamagePipelineGraphNode.h"
#include "Graph/DamagePipelineGraphNode.h"
#include "DamageFramework/DamageRule.h"
#include "DamageFramework/DamagePredicate.h"
#include "SGraphNode.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Layout/SBorder.h"
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

				// --- Body: Condition + Produces ---
				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(4.f)
				[
					SNew(SVerticalBox)

					+ SVerticalBox::Slot()
					.AutoHeight()
					[
						SNew(STextBlock)
						.Text(this, &SDamagePipelineGraphNode::GetConditionText)
						.ColorAndOpacity(FLinearColor(0.7f, 0.7f, 0.2f))
					]

					+ SVerticalBox::Slot()
					.AutoHeight()
					[
						SNew(STextBlock)
						.Text(this, &SDamagePipelineGraphNode::GetProducesText)
						.ColorAndOpacity(FLinearColor(0.4f, 0.7f, 0.4f))
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
				+ SVerticalBox::Slot()
				.AutoHeight()
				[
					SNew(SHorizontalBox)

					+ SHorizontalBox::Slot()
					.FillWidth(1.f)
					[
						SAssignNew(LeftNodeBox, SVerticalBox)
					]

					+ SHorizontalBox::Slot()
					.FillWidth(1.f)
					[
						SAssignNew(RightNodeBox, SVerticalBox)
					]
				]
			]
		];

	CreatePinWidgets();
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

FText SDamagePipelineGraphNode::GetConditionText() const
{
	if (PipelineGraphNode && PipelineGraphNode->Rule.IsValid())
	{
		if (PipelineGraphNode->Rule->Condition)
		{
			return FText::FromString(FString::Printf(
				TEXT("Cond: %s"), *PipelineGraphNode->Rule->Condition->GetDisplayString()));
		}
		return LOCTEXT("CondAlways", "Cond: (always)");
	}
	return FText::GetEmpty();
}

FText SDamagePipelineGraphNode::GetProducesText() const
{
	if (PipelineGraphNode && PipelineGraphNode->Rule.IsValid())
	{
		UScriptStruct* EffectType = PipelineGraphNode->Rule->GetProducesEffectType();
		if (EffectType)
		{
			return FText::FromString(FString::Printf(
				TEXT("Produces: %s"), *EffectType->GetName()));
		}
		return LOCTEXT("ProducesNone", "Produces: (none)");
	}
	return FText::GetEmpty();
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
