/***************************************************************************************************************
* Plugin:       SagaStats
* Author:       Claude(@JinmingZhang)
* Description:  SagaStats offers modular damage process and meter systems to support adaptable status management
****************************************************************************************************************/

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

	/**
	 * 静态：按 EffectType 查固定 12 色调色板（按类名哈希）。
	 * Pin 和 Condition 行的色块共用此函数确保色彩一致。
	 */
	static FLinearColor GetColorForEffectType(const class UScriptStruct* EffectType);

	// 自定义连线绘制策略：电路板风格 Manhattan 折线 + 通道化 DropX（配合阶梯式节点排列）
	virtual class FConnectionDrawingPolicy* CreateConnectionDrawingPolicy(
		int32 InBackLayerID, int32 InFrontLayerID, float InZoomFactor,
		const class FSlateRect& InClippingRect, class FSlateWindowElementList& InDrawElements,
		class UEdGraph* InGraphObj) const override;
};
