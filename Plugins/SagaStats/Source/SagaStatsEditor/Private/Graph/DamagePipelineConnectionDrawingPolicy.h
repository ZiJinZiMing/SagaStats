/***************************************************************************************************************
* Plugin:       SagaStats
* Author:       Claude(@JinmingZhang)
* Description:  SagaStats offers modular damage process and meter systems to support adaptable status management
****************************************************************************************************************/

// DamagePipelineConnectionDrawingPolicy.h — 电路板风格连线绘制策略
#pragma once

#include "CoreMinimal.h"
#include "ConnectionDrawingPolicy.h"

class UEdGraphSchema;

/**
 * 电路板风格连线绘制策略。
 *
 * 配合阶梯式节点排列 + 通道化 DropX，绘制是 3 段 Manhattan 折线：
 * - H 从源 pin 至 DropX
 * - V 从源 Y 至目标 Y
 * - H 至目标 pin
 *
 * DropX 由目标输入 Pin 的索引决定：DropX = DstRect.Left - (InputIdx+1) × ChannelWidth。
 * 每个输入 Pin 独占一条 X 通道；配合阶梯 Y 布局（每 Pin 全局唯一 Y），水平段 Y 和
 * 垂直段 X 都按构造不重合。不再需要避让或散布启发式。
 */
class FDamagePipelineConnectionDrawingPolicy : public FConnectionDrawingPolicy
{
public:
	FDamagePipelineConnectionDrawingPolicy(
		int32 InBackLayerID,
		int32 InFrontLayerID,
		float InZoomFactor,
		const FSlateRect& InClippingRect,
		FSlateWindowElementList& InDrawElements,
		const UEdGraphSchema* InSchema);

	// FConnectionDrawingPolicy
	virtual void Draw(TMap<TSharedRef<SWidget>, FArrangedWidget>& InPinGeometries, FArrangedChildren& ArrangedNodes) override;
	virtual void DetermineWiringStyle(UEdGraphPin* OutputPin, UEdGraphPin* InputPin, FConnectionParams& Params) override;
	virtual void DrawSplineWithArrow(const FGeometry& StartGeom, const FGeometry& EndGeom, const FConnectionParams& Params) override;
	virtual void DrawSplineWithArrow(const FVector2f& StartAnchorPoint, const FVector2f& EndAnchorPoint, const FConnectionParams& Params) override;
	virtual void DrawPreviewConnector(const FGeometry& PinGeometry, const FVector2f& StartPoint, const FVector2f& EndPoint, UEdGraphPin* Pin) override;
	virtual FVector2f ComputeSplineTangent(const FVector2f& Start, const FVector2f& End) const override;

private:
	/** 画折线并在终点位置画箭头 */
	void DrawManhattanPath(const TArray<FVector2D>& Path, const FConnectionParams& Params);

	const UEdGraphSchema* Schema = nullptr;

	/** 本帧节点矩形映射 (Slate absolute)。DrawSplineWithArrow 按目标节点查 rect，
	 *  然后基于目标输入 pin 索引算出专属 DropX 通道。 */
	TMap<class UDamagePipelineGraphNode*, FBox2D> NodeRectByObject;
};
