/***************************************************************************************************************
* Plugin:       SagaStats
* Author:       Claude(@JinmingZhang)
* Description:  SagaStats offers modular damage process and meter systems to support adaptable status management
****************************************************************************************************************/

// DamagePipelineGraph.cpp
#include "Graph/DamagePipelineGraph.h"
#include "Graph/DamagePipelineGraphNode.h"
#include "Graph/DamagePipelineGraphSchema.h"
#include "DamagePipeline/DamagePipeline.h"
#include "DamagePipeline/DamageRule.h"

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

	// ── 3. Create nodes + build producer map ──
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

	// ── 4. Create connections: match input pins to producer output pins by FName ──
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

	// 节点位置全部留空（0, 0）——真实布局由 FDamagePipelineAssetEditor::ApplyRealSizeLayoutCorrection
	// 在 Slate widget 构建完成后的一帧读真实尺寸后完成。
	// 这里的 RebuildGraph 只负责数据层：节点创建 + Pin 连接。
	for (UDamagePipelineGraphNode* Node : AllNodes)
	{
		Node->NodePosX = 0;
		Node->NodePosY = 0;
	}
}
