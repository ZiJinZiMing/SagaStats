/***************************************************************************************************************
* Plugin:       SagaStats
* Author:       Claude(@JinmingZhang)
* Description:  SagaStats offers modular damage process and meter systems to support adaptable status management
****************************************************************************************************************/

// DamagePipelineConnectionDrawingPolicy.cpp
#include "DamagePipelineConnectionDrawingPolicy.h"
#include "DamagePipelineLayoutConstants.h" // for DamagePipelineLayoutConstants::ChannelWidth
#include "Graph/DamagePipelineGraphNode.h"
#include "EdGraph/EdGraphSchema.h"
#include "EdGraph/EdGraphPin.h"
#include "Rendering/DrawElements.h"
#include "SGraphNode.h"

FDamagePipelineConnectionDrawingPolicy::FDamagePipelineConnectionDrawingPolicy(
	int32 InBackLayerID, int32 InFrontLayerID, float InZoomFactor,
	const FSlateRect& InClippingRect, FSlateWindowElementList& InDrawElements,
	const UEdGraphSchema* InSchema)
	: FConnectionDrawingPolicy(InBackLayerID, InFrontLayerID, InZoomFactor, InClippingRect, InDrawElements)
	, Schema(InSchema)
{
}

void FDamagePipelineConnectionDrawingPolicy::Draw(
	TMap<TSharedRef<SWidget>, FArrangedWidget>& InPinGeometries, FArrangedChildren& ArrangedNodes)
{
	// 本帧节点矩形映射（Slate absolute），按 UDamagePipelineGraphNode* 索引
	NodeRectByObject.Reset();
	NodeRectByObject.Reserve(ArrangedNodes.Num());
	for (int32 i = 0; i < ArrangedNodes.Num(); ++i)
	{
		const FArrangedWidget& Arranged = ArrangedNodes[i];
		TSharedRef<SGraphNode> GraphNodeWidget = StaticCastSharedRef<SGraphNode>(Arranged.Widget);
		UEdGraphNode* EdNode = GraphNodeWidget->GetNodeObj();
		UDamagePipelineGraphNode* DpNode = Cast<UDamagePipelineGraphNode>(EdNode);
		if (!DpNode) continue;

		const FVector2D Pos(Arranged.Geometry.GetAbsolutePosition());
		const FVector2D Size(Arranged.Geometry.GetAbsoluteSize());
		NodeRectByObject.Add(DpNode, FBox2D(Pos, Pos + Size));
	}

	FConnectionDrawingPolicy::Draw(InPinGeometries, ArrangedNodes);
}

void FDamagePipelineConnectionDrawingPolicy::DetermineWiringStyle(
	UEdGraphPin* OutputPin, UEdGraphPin* InputPin, FConnectionParams& Params)
{
	Params.AssociatedPin1 = OutputPin;
	Params.AssociatedPin2 = InputPin;
	Params.WireThickness = 2.5f;
	Params.bDrawBubbles = false;

	if (Schema && OutputPin)
	{
		Params.WireColor = Schema->GetPinTypeColor(OutputPin->PinType);
	}
}

void FDamagePipelineConnectionDrawingPolicy::DrawSplineWithArrow(
	const FGeometry& StartGeom, const FGeometry& EndGeom, const FConnectionParams& Params)
{
	const FVector2f StartAnchor = FGeometryHelper::VerticalMiddleRightOf(StartGeom);
	const FVector2f EndAnchor = FGeometryHelper::VerticalMiddleLeftOf(EndGeom);
	DrawSplineWithArrow(StartAnchor, EndAnchor, Params);
}

void FDamagePipelineConnectionDrawingPolicy::DrawSplineWithArrow(
	const FVector2f& StartAnchorPoint, const FVector2f& EndAnchorPoint, const FConnectionParams& Params)
{
	const FVector2D Start((double)StartAnchorPoint.X, (double)StartAnchorPoint.Y);
	const FVector2D End((double)EndAnchorPoint.X, (double)EndAnchorPoint.Y);

	const bool bSameY = FMath::IsNearlyEqual(Start.Y, End.Y, 0.5);

	TArray<FVector2D> PathSlate;

	if (bSameY)
	{
		PathSlate = { Start, End };
	}
	else
	{
		// 通道化 DropX：每个输入 pin 在目标节点左侧独占一条通道。
		// DropX = DstRect.Left - (InputIdx + 1) × ChannelWidth
		// 配合阶梯 Y 排列，不同目标节点的边的垂直段（X）和水平段（Y）不会重合。
		double DropX = (Start.X + End.X) * 0.5; // fallback

		UDamagePipelineGraphNode* DstNode = nullptr;
		UEdGraphPin* InputPin = Params.AssociatedPin2;
		if (InputPin)
		{
			DstNode = Cast<UDamagePipelineGraphNode>(InputPin->GetOwningNode());
		}

		if (DstNode && InputPin)
		{
			// 找到该输入 Pin 在目标节点输入序列中的 index
			int32 InputIdx = 0;
			int32 Counter = 0;
			for (UEdGraphPin* P : DstNode->Pins)
			{
				if (P->Direction != EGPD_Input) continue;
				if (P == InputPin) { InputIdx = Counter; break; }
				++Counter;
			}

			if (const FBox2D* DstRect = NodeRectByObject.Find(DstNode))
			{
				DropX = DstRect->Min.X - (double)(InputIdx + 1) * (double)DamagePipelineLayoutConstants::ChannelWidth;
			}
		}

		PathSlate = {
			Start,
			FVector2D(DropX, Start.Y),
			FVector2D(DropX, End.Y),
			End
		};
	}

	DrawManhattanPath(PathSlate, Params);
}

void FDamagePipelineConnectionDrawingPolicy::DrawPreviewConnector(
	const FGeometry& PinGeometry, const FVector2f& StartPoint, const FVector2f& EndPoint, UEdGraphPin* Pin)
{
	FConnectionParams Params;
	DetermineWiringStyle(Pin, nullptr, Params);
	DrawSplineWithArrow(StartPoint, EndPoint, Params);
}

FVector2f FDamagePipelineConnectionDrawingPolicy::ComputeSplineTangent(const FVector2f& Start, const FVector2f& End) const
{
	return FVector2f(1.0f, 0.0f);
}

void FDamagePipelineConnectionDrawingPolicy::DrawManhattanPath(
	const TArray<FVector2D>& Path, const FConnectionParams& Params)
{
	if (Path.Num() < 2) return;

	FSlateDrawElement::MakeLines(
		DrawElementsList,
		WireLayerID,
		FPaintGeometry(),
		Path,
		ESlateDrawEffect::None,
		Params.WireColor,
		/*bAntialias=*/true,
		Params.WireThickness);

	// 终点箭头
	if (ArrowImage)
	{
		const FVector2D& ArrowTip = Path.Last();
		const FVector2D& Prev = Path[Path.Num() - 2];
		const FVector2D Dir = (ArrowTip - Prev).GetSafeNormal();
		const float AngleRad = FMath::Atan2(Dir.Y, Dir.X);

		const FVector2f ArrowSize(ArrowImage->ImageSize.X, ArrowImage->ImageSize.Y);
		const FVector2f ArrowPos(
			(float)ArrowTip.X - ArrowSize.X * 0.5f,
			(float)ArrowTip.Y - ArrowSize.Y * 0.5f);

		FSlateDrawElement::MakeRotatedBox(
			DrawElementsList,
			ArrowLayerID,
			FPaintGeometry(ArrowPos, ArrowSize, 1.0f),
			ArrowImage,
			ESlateDrawEffect::None,
			AngleRad,
			ArrowSize * 0.5f,
			FSlateDrawElement::RelativeToElement,
			Params.WireColor);
	}
}
