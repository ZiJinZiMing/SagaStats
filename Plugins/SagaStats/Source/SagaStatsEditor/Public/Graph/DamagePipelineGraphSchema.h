// DamagePipelineGraphSchema.h — 只读 DAG 图表的 Schema
#pragma once

#include "CoreMinimal.h"
#include "EdGraph/EdGraphSchema.h"
#include "DamagePipelineGraphSchema.generated.h"

UCLASS()
class UDamagePipelineGraphSchema : public UEdGraphSchema
{
	GENERATED_BODY()

public:
	// 只读图表：禁止手动连线
	virtual const FPinConnectionResponse CanCreateConnection(const UEdGraphPin* A, const UEdGraphPin* B) const override;

	// 只读图表：禁止右键菜单添加节点
	virtual void GetContextMenuActions(UToolMenu* Menu, UGraphNodeContextMenuContext* Context) const override {}

	// Pin 颜色
	virtual FLinearColor GetPinTypeColor(const FEdGraphPinType& PinType) const override;
};
