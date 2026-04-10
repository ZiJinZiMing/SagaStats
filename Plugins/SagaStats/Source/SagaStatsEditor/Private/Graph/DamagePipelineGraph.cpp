// DamagePipelineGraph.cpp
#include "Graph/DamagePipelineGraph.h"
#include "Graph/DamagePipelineGraphNode.h"
#include "Graph/DamagePipelineGraphSchema.h"
#include "DamageFramework/DamagePipeline.h"
#include "DamageFramework/DamageRule.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(DamagePipelineGraph)

void UDamagePipelineGraph::RebuildGraph(UDamagePipeline* Pipeline)
{
	if (!Pipeline) return;

	// ── 1. Build if not baked ──
	if (!Pipeline->bIsBaked)
	{
		Pipeline->Build();
	}

	// ── 2. Clear existing nodes ──
	Modify();
	TArray<UEdGraphNode*> OldNodes = Nodes;
	for (UEdGraphNode* Node : OldNodes)
	{
		RemoveNode(Node);
	}

	// ── 3. Get sorted rules via Build() ──
	FPipelineSortResult SortResult = Pipeline->Build();
	if (SortResult.bHasCycle)
	{
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

	// ── 6. Auto-layout: left-to-right by topological layer ──
	const int32 NodeCount = AllNodes.Num();
	if (NodeCount == 0) return;

	// Calculate layer for each node (max layer of dependencies + 1)
	TArray<int32> Layers;
	Layers.SetNumZeroed(NodeCount);

	// Build node-to-index map
	TMap<UDamagePipelineGraphNode*, int32> NodeToIndex;
	for (int32 i = 0; i < NodeCount; ++i)
	{
		NodeToIndex.Add(AllNodes[i], i);
	}

	// Calculate layers: iterate in topological order (SortedRules is already sorted)
	for (int32 i = 0; i < NodeCount; ++i)
	{
		int32 MaxDepLayer = -1;

		// Check all input pins for dependencies
		for (UEdGraphPin* InputPin : AllNodes[i]->Pins)
		{
			if (InputPin->Direction != EGPD_Input) continue;

			for (UEdGraphPin* LinkedPin : InputPin->LinkedTo)
			{
				UDamagePipelineGraphNode* DepNode = Cast<UDamagePipelineGraphNode>(LinkedPin->GetOwningNode());
				if (const int32* DepIdx = NodeToIndex.Find(DepNode))
				{
					MaxDepLayer = FMath::Max(MaxDepLayer, Layers[*DepIdx]);
				}
			}
		}

		Layers[i] = MaxDepLayer + 1;
	}

	// Group by layer and assign positions
	constexpr float XSpacing = 400.f;
	constexpr float YSpacing = 200.f;

	TMap<int32, int32> LayerCount; // layer -> current count in that layer
	for (int32 i = 0; i < NodeCount; ++i)
	{
		int32 Layer = Layers[i];
		int32& YIndex = LayerCount.FindOrAdd(Layer, 0);

		AllNodes[i]->NodePosX = Layer * XSpacing;
		AllNodes[i]->NodePosY = YIndex * YSpacing;

		++YIndex;
	}
}
