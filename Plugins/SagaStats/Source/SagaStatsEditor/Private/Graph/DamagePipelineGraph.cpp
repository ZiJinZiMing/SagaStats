// DamagePipelineGraph.cpp
#include "Graph/DamagePipelineGraph.h"
#include "Graph/DamagePipelineGraphNode.h"
#include "Graph/DamagePipelineGraphSchema.h"
#include "DamagePipelineLayoutEngine.h"
#include "DamageFramework/DamagePipeline.h"
#include "DamageFramework/DamageRule.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(DamagePipelineGraph)

void UDamagePipelineGraph::RebuildGraph(UDamagePipeline* Pipeline)
{
	if (!Pipeline) return;

	// ── 1. Clear existing nodes ──
	Modify();
	TArray<UEdGraphNode*> OldNodes = Nodes;
	for (UEdGraphNode* Node : OldNodes)
	{
		RemoveNode(Node);
	}

	// ── 2. Get sorted rules via Build()（内部有缓存，重复调用不会重算） ──
	FPipelineSortResult SortResult = Pipeline->Build();
	if (SortResult.bHasCycle)
	{
		UE_LOG(LogTemp, Error, TEXT("[DamagePipeline] RebuildGraph aborted: cycle detected in pipeline '%s'"),
			*Pipeline->GetName());
		return;
	}

	// Set schema
	Schema = UDamagePipelineGraphSchema::StaticClass();

	// ── 4. Create nodes + build producer map ──
	// Producer map: ProducesEffectType -> Node that produces it
	TMap<UScriptStruct*, UDamagePipelineGraphNode*> ProducerMap;

	TArray<UDamagePipelineGraphNode*> AllNodes;
	AllNodes.Reserve(SortResult.SortedRules.Num());

	for (int32 Index = 0; Index < SortResult.SortedRules.Num(); ++Index)
	{
		UDamageRule* Rule = SortResult.SortedRules[Index];
		if (!Rule) continue;

		UDamagePipelineGraphNode* Node = NewObject<UDamagePipelineGraphNode>(this);
		Node->SetupNode(Rule, Index);
		Node->CreateNewGuid();
		Node->PostPlacedNewNode();
		Node->AllocateDefaultPins();

		AddNode(Node, /*bUserAction=*/false, /*bSelectNewNode=*/false);
		AllNodes.Add(Node);

		// Register in producer map
		UScriptStruct* ProducesType = Rule->GetProducesEffectType();
		if (ProducesType)
		{
			ProducerMap.Add(ProducesType, Node);
		}
	}

	// ── 5. Create connections: match input pins to producer output pins by FName ──
	for (UDamagePipelineGraphNode* ConsumerNode : AllNodes)
	{
		for (UEdGraphPin* InputPin : ConsumerNode->Pins)
		{
			if (InputPin->Direction != EGPD_Input) continue;

			// Find producer node whose output pin Name matches this input pin Name
			for (auto& Pair : ProducerMap)
			{
				UDamagePipelineGraphNode* ProducerNode = Pair.Value;
				for (UEdGraphPin* OutputPin : ProducerNode->Pins)
				{
					if (OutputPin->Direction != EGPD_Output) continue;
					if (OutputPin->PinName == InputPin->PinName)
					{
						InputPin->MakeLinkTo(OutputPin);
					}
				}
			}
		}
	}

	// ── 6. 阶梯 + 通道布局 ──
	// Y 方向：每节点累积 Pin 行数 → 每 Pin 全局唯一 Y
	// X 方向：每节点前的 gap 根据该节点输入数扩展 → 每输入独占一条垂直通道
	FDamagePipelineLayoutInput LayoutInput;
	LayoutInput.TotalPinCounts.Reserve(AllNodes.Num());
	LayoutInput.InputPinCounts.Reserve(AllNodes.Num());
	for (UDamagePipelineGraphNode* Node : AllNodes)
	{
		LayoutInput.TotalPinCounts.Add(Node->Pins.Num());

		int32 InputCount = 0;
		for (UEdGraphPin* Pin : Node->Pins)
		{
			if (Pin->Direction == EGPD_Input)
			{
				++InputCount;
			}
		}
		LayoutInput.InputPinCounts.Add(InputCount);
	}

	const FDamagePipelineLayoutOutput LayoutOutput =
		FDamagePipelineLayoutEngine::Compute(LayoutInput);

	for (int32 i = 0; i < AllNodes.Num(); ++i)
	{
		if (LayoutOutput.RuleNodePositions.IsValidIndex(i))
		{
			AllNodes[i]->NodePosX = LayoutOutput.RuleNodePositions[i].X;
			AllNodes[i]->NodePosY = LayoutOutput.RuleNodePositions[i].Y;
		}
	}
}
