// SDamagePipelineGraphNode.cpp
#include "Graph/SDamagePipelineGraphNode.h"
#include "Graph/DamagePipelineGraphNode.h"
#include "DamageFramework/DamageRule.h"
#include "DamageFramework/DamagePredicate.h"
#include "SGraphNode.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Layout/SBorder.h"
#include "Styling/AppStyle.h"

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
	if (PipelineGraphNode && PipelineGraphNode->Rule.IsValid())
	{
		return FText::FromString(FString::Printf(
			TEXT("#%d %s"),
			PipelineGraphNode->SortIndex,
			*PipelineGraphNode->Rule->RuleName.ToString()));
	}
	return LOCTEXT("InvalidNode", "(Invalid)");
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
