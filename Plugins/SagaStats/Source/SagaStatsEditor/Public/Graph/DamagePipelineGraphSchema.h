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

	// 自定义连线绘制策略：电路板风格 Manhattan 折线 + 通道化 DropX（配合阶梯布局）
	virtual class FConnectionDrawingPolicy* CreateConnectionDrawingPolicy(
		int32 InBackLayerID, int32 InFrontLayerID, float InZoomFactor,
		const class FSlateRect& InClippingRect, class FSlateWindowElementList& InDrawElements,
		class UEdGraph* InGraphObj) const override;
};
