/***************************************************************************************************************
* Plugin:       SagaStats
* Author:       Claude(@JinmingZhang)
* Description:  SagaStats offers modular damage process and meter systems to support adaptable status management
****************************************************************************************************************/

// DamagePipelineGraphNode.h — DamageRule 的图表节点
#pragma once

#include "CoreMinimal.h"
#include "EdGraph/EdGraphNode.h"
#include "DamagePipelineGraphNode.generated.h"

class UDamageRule;

UCLASS()
class UDamagePipelineGraphNode : public UEdGraphNode
{
	GENERATED_BODY()

public:
	UPROPERTY()
	TWeakObjectPtr<UDamageRule> Rule;

	/** 节点在拓扑排序中的序号 */
	UPROPERTY()
	int32 SortIndex = -1;

	void SetupNode(UDamageRule* InRule, int32 InSortIndex);

	// UEdGraphNode
	virtual FText GetNodeTitle(ENodeTitleType::Type TitleType) const override;
	virtual FText GetTooltipText() const override;
	virtual void AllocateDefaultPins() override;
	virtual bool CanUserDeleteNode() const override { return false; }
	virtual bool CanDuplicateNode() const override { return false; }
};
