// DamagePipelineGraphNode.cpp
#include "Graph/DamagePipelineGraphNode.h"
#include "DamageFramework/DamageRule.h"
#include "DamageFramework/DamagePredicate.h"

#define LOCTEXT_NAMESPACE "DamagePipelineGraphNode"

void UDamagePipelineGraphNode::SetupNode(UDamageRule* InRule, int32 InSortIndex)
{
	Rule = InRule;
	SortIndex = InSortIndex;
}

FText UDamagePipelineGraphNode::GetNodeTitle(ENodeTitleType::Type TitleType) const
{
	if (!Rule.IsValid())
	{
		return LOCTEXT("InvalidNode", "(Invalid)");
	}
	return FText::FromString(FString::Printf(TEXT("%d. %s"), SortIndex + 1, *Rule->GetName()));
}

FText UDamagePipelineGraphNode::GetTooltipText() const
{
	if (!Rule.IsValid()) return FText::GetEmpty();

	FString Tip;

	// Condition
	if (Rule->Condition)
	{
		Tip += FString::Printf(TEXT("Cond: %s\n"), *Rule->Condition->GetDisplayString());
	}
	else
	{
		Tip += TEXT("Cond: (always)\n");
	}

	// Produces
	UScriptStruct* EffectType = Rule->GetProducesEffectType();
	if (EffectType)
	{
		Tip += FString::Printf(TEXT("Produces: %s"), *EffectType->GetName());
	}

	return FText::FromString(Tip);
}

void UDamagePipelineGraphNode::AllocateDefaultPins()
{
	if (!Rule.IsValid()) return;

	// 输出 Pin：此 Rule 产出的 Effect 类型
	UScriptStruct* ProducesType = Rule->GetProducesEffectType();
	if (ProducesType)
	{
		UEdGraphPin* OutPin = CreatePin(EGPD_Output, TEXT("DamageEffect"), ProducesType->GetFName());
		OutPin->PinType.PinSubCategoryObject = ProducesType;
	}

	// 输入 Pin：此 Rule 依赖的 Effect 类型（每个 ConsumedEffectType 一个 Pin）
	TArray<UScriptStruct*> ConsumedTypes = Rule->GetConsumedEffectTypes();
	for (UScriptStruct* Type : ConsumedTypes)
	{
		if (Type)
		{
			UEdGraphPin* InPin = CreatePin(EGPD_Input, TEXT("DamageEffect"), Type->GetFName());
			InPin->PinType.PinSubCategoryObject = Type;
		}
	}
}

#undef LOCTEXT_NAMESPACE
