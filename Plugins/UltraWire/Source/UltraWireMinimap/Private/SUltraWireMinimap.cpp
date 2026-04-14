// Copyright 2026 StraySpark. All Rights Reserved.

#include "SUltraWireMinimap.h"
#include "UltraWireSettings.h"
#include "UltraWireTypes.h"

#include "SGraphPanel.h"
#include "EdGraph/EdGraph.h"
#include "EdGraph/EdGraphNode.h"
#include "EdGraph/EdGraphPin.h"
// We avoid including EdGraphSchema_K2.h and linking BlueprintGraph.
// Instead we use FName string literals matching the K2 pin category names.

#include "Rendering/DrawElements.h"
#include "Styling/AppStyle.h"
#include "Styling/CoreStyle.h"
#include "Styling/SlateBrush.h"
#include "Brushes/SlateColorBrush.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Images/SImage.h"
#include "Framework/Application/SlateApplication.h"
#include "Input/Events.h"
#include "Layout/WidgetPath.h"

// ---------------------------------------------------------------------------
// Module-internal constants
// ---------------------------------------------------------------------------
namespace
{
	/** Background colour of the minimap panel (RGBA). */
	static const FLinearColor MinimapBackgroundColor(0.05f, 0.05f, 0.05f, 0.85f);

	/** Colour used for the viewport indicator border. */
	static const FLinearColor ViewportIndicatorColor(1.0f, 1.0f, 1.0f, 0.6f);

	/** Colour used for wire lines. */
	static const FLinearColor DefaultWireColor(0.5f, 0.5f, 0.5f, 0.6f);

	/** Colour used for regular node rectangles when no type colour is available. */
	static const FLinearColor DefaultNodeColor(0.3f, 0.3f, 0.35f, 0.9f);

	/** Colour used for comment node outlines. */
	static const FLinearColor CommentOutlineColor(0.8f, 0.8f, 0.2f, 0.7f);

	/** Corner radius of the minimap background in pixels. */
	static constexpr float BackgroundCornerRadius = 6.0f;

	/** Thickness of wire lines in the minimap in pixels. */
	static constexpr float WireLineThickness = 0.5f;

	/** Thickness of the viewport indicator border in pixels. */
	static constexpr float ViewportBorderThickness = 1.0f;

	/** Thickness of comment node outlines in pixels. */
	static constexpr float CommentOutlineThickness = 1.0f;
}

// ---------------------------------------------------------------------------
// Construct
// ---------------------------------------------------------------------------

void SUltraWireMinimap::Construct(const FArguments& InArgs)
{
	WeakGraphPanel = InArgs._GraphPanel;

	// Perform an initial topology snapshot so the minimap is not blank on
	// the first paint.
	CacheTopology();

	// The minimap widget itself has no Slate children – all rendering is done
	// in OnPaint() using low-level draw element calls.
	ChildSlot
	[
		SNew(SBox)
		// Size is driven by ComputeDesiredSize; SBox is here purely to
		// satisfy the compound-widget requirement for a child slot.
	];
}

// ---------------------------------------------------------------------------
// SWidget interface – desired size
// ---------------------------------------------------------------------------

FVector2D SUltraWireMinimap::ComputeDesiredSize(float LayoutScaleMultiplier) const
{
	const float Size = GetMinimapSizeFromSettings();
	return FVector2D(Size, Size);
}

// ---------------------------------------------------------------------------
// SWidget interface – tick
// ---------------------------------------------------------------------------

void SUltraWireMinimap::Tick(const FGeometry& AllottedGeometry,
                              const double InCurrentTime,
                              const float InDeltaTime)
{
	SCompoundWidget::Tick(AllottedGeometry, InCurrentTime, InDeltaTime);

	TopologyUpdateAccumulator += InDeltaTime;
	if (TopologyUpdateAccumulator >= TopologyUpdateInterval)
	{
		TopologyUpdateAccumulator = 0.0f;
		CacheTopology();
	}
}

// ---------------------------------------------------------------------------
// SWidget interface – paint
// ---------------------------------------------------------------------------

int32 SUltraWireMinimap::OnPaint(const FPaintArgs& Args,
                                  const FGeometry& AllottedGeometry,
                                  const FSlateRect& MyCullingRect,
                                  FSlateWindowElementList& OutDrawElements,
                                  int32 LayerId,
                                  const FWidgetStyle& InWidgetStyle,
                                  bool bParentEnabled) const
{
	// Read current settings.
	const UUltraWireSettings* Settings = UUltraWireSettings::Get();
	if (!Settings || !Settings->bEnabled || !Settings->DefaultProfile.bEnableMinimap)
	{
		return LayerId;
	}

	const float Opacity = GetMinimapOpacityFromSettings();

	// Layer stack (bottom to top):
	//   LayerId + 0 : background
	//   LayerId + 1 : wire lines
	//   LayerId + 2 : node rectangles
	//   LayerId + 3 : viewport indicator
	PaintBackground(AllottedGeometry, OutDrawElements, LayerId,     Opacity);
	PaintWires     (AllottedGeometry, OutDrawElements, LayerId + 1, Opacity);
	PaintNodes     (AllottedGeometry, OutDrawElements, LayerId + 2, Opacity);
	PaintViewport  (AllottedGeometry, OutDrawElements, LayerId + 3, Opacity);

	// Let child widgets paint on top (SBox in our slot is empty, but the
	// call must still happen per SCompoundWidget contract).
	return SCompoundWidget::OnPaint(Args, AllottedGeometry, MyCullingRect,
	                                OutDrawElements, LayerId + 4,
	                                InWidgetStyle, bParentEnabled);
}

// ---------------------------------------------------------------------------
// SWidget interface – mouse input
// ---------------------------------------------------------------------------

FReply SUltraWireMinimap::OnMouseButtonDown(const FGeometry& MyGeometry,
                                              const FPointerEvent& MouseEvent)
{
	if (MouseEvent.GetEffectingButton() != EKeys::LeftMouseButton)
	{
		return FReply::Unhandled();
	}

	TSharedPtr<SGraphPanel> GraphPanel = WeakGraphPanel.Pin();
	if (!GraphPanel.IsValid())
	{
		return FReply::Unhandled();
	}

	// Convert click position from absolute screen space to minimap-local space.
	const FVector2D LocalClickPos = MyGeometry.AbsoluteToLocal(
		MouseEvent.GetScreenSpacePosition());

	const FVector2D MinimapSize = MyGeometry.GetLocalSize();

	// Convert minimap-local click to graph space.
	const FVector2D GraphTargetPos = TransformMinimapToGraph(LocalClickPos, MinimapSize);

	// Pan the graph panel so that the clicked location is centred.
	// SGraphPanel exposes a JumpToNode / ScrollToLocation style function via
	// the ZoomToFit mechanism.  The most portable approach is to set the
	// view offset directly.  SGraphPanel inherits from SNodePanel which
	// exposes RestoreViewSettings().
	//
	// Pan to centre the target graph position in the visible area.  The
	// panel's geometry size gives us the visible extents.
	const FVector2D PanelLocalSize = GraphPanel->GetCachedGeometry().GetLocalSize();
	const float ZoomAmount = GraphPanel->GetZoomAmount();

	// Desired view offset so that GraphTargetPos appears at the panel centre.
	const FVector2D NewViewOffset = GraphTargetPos - (PanelLocalSize * 0.5f / ZoomAmount);
	GraphPanel->RestoreViewSettings(NewViewOffset, ZoomAmount);

	return FReply::Handled();
}

// ---------------------------------------------------------------------------
// Topology caching
// ---------------------------------------------------------------------------

void SUltraWireMinimap::CacheTopology()
{
	CachedNodes.Reset();
	CachedWires.Reset();

	TSharedPtr<SGraphPanel> GraphPanel = WeakGraphPanel.Pin();
	if (!GraphPanel.IsValid())
	{
		return;
	}

	// Obtain the UEdGraph from the graph panel.
	// SGraphPanel exposes GetGraphObj() which returns the UEdGraph pointer.
	UEdGraph* Graph = GraphPanel->GetGraphObj();
	if (!Graph)
	{
		return;
	}

	// Pre-size the arrays.
	CachedNodes.Reserve(Graph->Nodes.Num());

	for (UEdGraphNode* Node : Graph->Nodes)
	{
		if (!Node)
		{
			continue;
		}

		FMinimapNodeEntry Entry;
		Entry.GraphPosition = FVector2D(static_cast<float>(Node->NodePosX),
		                               static_cast<float>(Node->NodePosY));

		// Use the node's estimated size.  GetNodeBounds() is unavailable from
		// this context, but the schema provides a default width/height that
		// works well enough for minimap purposes.
		Entry.GraphSize = FVector2D(Node->NodeWidth  > 0 ? static_cast<float>(Node->NodeWidth)  : 200.0f,
		                            Node->NodeHeight > 0 ? static_cast<float>(Node->NodeHeight) : 80.0f);

		Entry.Color = FLinearColor(Node->GetNodeTitleColor());
		Entry.Color.A = 0.9f;

		// Detect comment nodes by class name to avoid a hard dependency on
		// UEdGraphNode_Comment (which lives in a different module).
		Entry.bIsComment = Node->GetClass()->GetFName() == FName(TEXT("EdGraphNode_Comment"));

		CachedNodes.Add(Entry);

		// Cache wires: for each output pin, find connected input pins and
		// store the start/end positions.
		for (UEdGraphPin* Pin : Node->Pins)
		{
			if (!Pin || Pin->Direction != EGPD_Output)
			{
				continue;
			}

			for (UEdGraphPin* LinkedPin : Pin->LinkedTo)
			{
				if (!LinkedPin || !LinkedPin->GetOwningNode())
				{
					continue;
				}

				UEdGraphNode* TargetNode = LinkedPin->GetOwningNode();

				FMinimapWireEntry Wire;
				// Start at the right edge of the source node (approximate).
				Wire.StartGraphPos = Entry.GraphPosition +
				                     FVector2D(Entry.GraphSize.X, Entry.GraphSize.Y * 0.5f);
				// End at the left edge of the target node (approximate).
				Wire.EndGraphPos   = FVector2D(static_cast<float>(TargetNode->NodePosX),
				                              static_cast<float>(TargetNode->NodePosY) +
				                              (TargetNode->NodeHeight > 0
				                               ? static_cast<float>(TargetNode->NodeHeight) * 0.5f
				                               : 40.0f));

				// Derive a colour from the pin's type using string name
				// comparisons to avoid linking against BlueprintGraph.
				Wire.Color = DefaultWireColor;
				const FName PinCat = Pin->PinType.PinCategory;
				if (PinCat == TEXT("exec"))
				{
					Wire.Color = FLinearColor(1.0f, 1.0f, 1.0f, 0.5f);
				}
				else if (PinCat == TEXT("bool"))
				{
					Wire.Color = FLinearColor(0.7f, 0.0f, 0.0f, 0.6f);
				}
				else if (PinCat == TEXT("int") || PinCat == TEXT("int64"))
				{
					Wire.Color = FLinearColor(0.2f, 0.6f, 1.0f, 0.6f);
				}
				else if (PinCat == TEXT("float") || PinCat == TEXT("double") || PinCat == TEXT("real"))
				{
					Wire.Color = FLinearColor(0.2f, 0.8f, 0.2f, 0.6f);
				}
				else if (PinCat == TEXT("object") || PinCat == TEXT("interface"))
				{
					Wire.Color = FLinearColor(0.0f, 0.5f, 1.0f, 0.6f);
				}
				else if (PinCat == TEXT("struct"))
				{
					Wire.Color = FLinearColor(0.0f, 0.7f, 0.7f, 0.6f);
				}

				CachedWires.Add(Wire);
			}
		}
	}

	RecalculateGraphBounds();
}

void SUltraWireMinimap::RecalculateGraphBounds()
{
	if (CachedNodes.Num() == 0)
	{
		GraphBoundsMin = FVector2D(-500.0f, -500.0f);
		GraphBoundsMax = FVector2D( 500.0f,  500.0f);
		return;
	}

	FVector2D MinBound( FLT_MAX,  FLT_MAX);
	FVector2D MaxBound(-FLT_MAX, -FLT_MAX);

	for (const FMinimapNodeEntry& Node : CachedNodes)
	{
		MinBound.X = FMath::Min(MinBound.X, Node.GraphPosition.X);
		MinBound.Y = FMath::Min(MinBound.Y, Node.GraphPosition.Y);
		MaxBound.X = FMath::Max(MaxBound.X, Node.GraphPosition.X + Node.GraphSize.X);
		MaxBound.Y = FMath::Max(MaxBound.Y, Node.GraphPosition.Y + Node.GraphSize.Y);
	}

	// Add padding so nodes at the boundary are not clipped.
	MinBound -= FVector2D(BoundsPaddingGraphUnits, BoundsPaddingGraphUnits);
	MaxBound += FVector2D(BoundsPaddingGraphUnits, BoundsPaddingGraphUnits);

	GraphBoundsMin = MinBound;
	GraphBoundsMax = MaxBound;
}

// ---------------------------------------------------------------------------
// Coordinate transforms
// ---------------------------------------------------------------------------

FVector2D SUltraWireMinimap::TransformGraphToMinimap(FVector2D GraphPos,
                                                      FVector2D MinimapSize) const
{
	const FVector2D GraphExtent = GraphBoundsMax - GraphBoundsMin;

	// Avoid divide-by-zero when the graph is empty.
	if (GraphExtent.X < 1.0f || GraphExtent.Y < 1.0f)
	{
		return MinimapSize * 0.5f;
	}

	// Maintain aspect ratio by using uniform scaling and centering.
	const float ScaleX = MinimapSize.X / GraphExtent.X;
	const float ScaleY = MinimapSize.Y / GraphExtent.Y;
	const float Scale  = FMath::Min(ScaleX, ScaleY);

	const FVector2D ScaledExtent = GraphExtent * Scale;
	const FVector2D Offset       = (MinimapSize - ScaledExtent) * 0.5f;

	return (GraphPos - GraphBoundsMin) * Scale + Offset;
}

FVector2D SUltraWireMinimap::TransformMinimapToGraph(FVector2D MinimapPos,
                                                      FVector2D MinimapSize) const
{
	const FVector2D GraphExtent = GraphBoundsMax - GraphBoundsMin;

	if (GraphExtent.X < 1.0f || GraphExtent.Y < 1.0f)
	{
		return FVector2D::ZeroVector;
	}

	const float ScaleX = MinimapSize.X / GraphExtent.X;
	const float ScaleY = MinimapSize.Y / GraphExtent.Y;
	const float Scale  = FMath::Min(ScaleX, ScaleY);

	const FVector2D ScaledExtent = GraphExtent * Scale;
	const FVector2D Offset       = (MinimapSize - ScaledExtent) * 0.5f;

	return (MinimapPos - Offset) / Scale + GraphBoundsMin;
}

// ---------------------------------------------------------------------------
// Settings accessors
// ---------------------------------------------------------------------------

float SUltraWireMinimap::GetMinimapSizeFromSettings() const
{
	if (const UUltraWireSettings* Settings = UUltraWireSettings::Get())
	{
		return Settings->DefaultProfile.MinimapSize;
	}
	return 200.0f;
}

float SUltraWireMinimap::GetMinimapOpacityFromSettings() const
{
	if (const UUltraWireSettings* Settings = UUltraWireSettings::Get())
	{
		return Settings->DefaultProfile.MinimapOpacity;
	}
	return 0.8f;
}

// ---------------------------------------------------------------------------
// Rendering helpers
// ---------------------------------------------------------------------------

void SUltraWireMinimap::PaintBackground(const FGeometry& AllottedGeometry,
                                         FSlateWindowElementList& OutDrawElements,
                                         int32 LayerId,
                                         float Opacity) const
{
	const FVector2D LocalSize = AllottedGeometry.GetLocalSize();

	FLinearColor BgColor = MinimapBackgroundColor;
	BgColor.A *= Opacity;

	// Draw a solid rectangle as the background.
	const FSlateBrush* WhiteBrush = FCoreStyle::Get().GetBrush("WhiteBrush");
	FSlateDrawElement::MakeBox(
		OutDrawElements,
		LayerId,
		AllottedGeometry.ToPaintGeometry(LocalSize, FSlateLayoutTransform()),
		WhiteBrush,
		ESlateDrawEffect::None,
		BgColor);
}

void SUltraWireMinimap::PaintNodes(const FGeometry& AllottedGeometry,
                                    FSlateWindowElementList& OutDrawElements,
                                    int32 LayerId,
                                    float Opacity) const
{
	const FVector2D MinimapSize = AllottedGeometry.GetLocalSize();

	for (const FMinimapNodeEntry& Node : CachedNodes)
	{
		const FVector2D MinimapTopLeft = TransformGraphToMinimap(Node.GraphPosition, MinimapSize);
		const FVector2D GraphExtent    = GraphBoundsMax - GraphBoundsMin;

		// Compute the uniform scale factor used in TransformGraphToMinimap.
		float Scale = 1.0f;
		if (GraphExtent.X > 1.0f && GraphExtent.Y > 1.0f)
		{
			Scale = FMath::Min(MinimapSize.X / GraphExtent.X,
			                   MinimapSize.Y / GraphExtent.Y);
		}

		FVector2D MinimapNodeSize = Node.GraphSize * Scale;

		// Clamp to minimum visible size.
		MinimapNodeSize.X = FMath::Max(MinimapNodeSize.X, MinNodeRectSizePixels);
		MinimapNodeSize.Y = FMath::Max(MinimapNodeSize.Y, MinNodeRectSizePixels);

		FLinearColor DrawColor = Node.Color;
		DrawColor.A *= Opacity;

		if (Node.bIsComment)
		{
			// Comment nodes: outline only.
			FLinearColor OutlineColor = CommentOutlineColor;
			OutlineColor.A *= Opacity;

			FSlateDrawElement::MakeLines(
				OutDrawElements,
				LayerId,
				AllottedGeometry.ToPaintGeometry(),
				TArray<FVector2D>{
					MinimapTopLeft,
					MinimapTopLeft + FVector2D(MinimapNodeSize.X, 0.0f),
					MinimapTopLeft + MinimapNodeSize,
					MinimapTopLeft + FVector2D(0.0f, MinimapNodeSize.Y),
					MinimapTopLeft
				},
				ESlateDrawEffect::None,
				OutlineColor,
				true,
				CommentOutlineThickness);
		}
		else
		{
			// Regular nodes: filled rectangle.
			const FSlateBrush* NodeBrush = FCoreStyle::Get().GetBrush("WhiteBrush");
			FSlateDrawElement::MakeBox(
				OutDrawElements,
				LayerId,
				AllottedGeometry.ToPaintGeometry(MinimapNodeSize,
				                                 FSlateLayoutTransform(MinimapTopLeft)),
				NodeBrush,
				ESlateDrawEffect::None,
				DrawColor);
		}
	}
}

void SUltraWireMinimap::PaintWires(const FGeometry& AllottedGeometry,
                                    FSlateWindowElementList& OutDrawElements,
                                    int32 LayerId,
                                    float Opacity) const
{
	const FVector2D MinimapSize = AllottedGeometry.GetLocalSize();

	for (const FMinimapWireEntry& Wire : CachedWires)
	{
		const FVector2D StartMM = TransformGraphToMinimap(Wire.StartGraphPos, MinimapSize);
		const FVector2D EndMM   = TransformGraphToMinimap(Wire.EndGraphPos,   MinimapSize);

		FLinearColor WireColor = Wire.Color;
		WireColor.A *= Opacity;

		FSlateDrawElement::MakeLines(
			OutDrawElements,
			LayerId,
			AllottedGeometry.ToPaintGeometry(),
			TArray<FVector2D>{ StartMM, EndMM },
			ESlateDrawEffect::None,
			WireColor,
			true,
			WireLineThickness);
	}
}

void SUltraWireMinimap::PaintViewport(const FGeometry& AllottedGeometry,
                                       FSlateWindowElementList& OutDrawElements,
                                       int32 LayerId,
                                       float Opacity) const
{
	TSharedPtr<SGraphPanel> GraphPanel = WeakGraphPanel.Pin();
	if (!GraphPanel.IsValid())
	{
		return;
	}

	const FVector2D MinimapSize = AllottedGeometry.GetLocalSize();

	// Obtain the current view offset and zoom from the graph panel.
	// SNodePanel exposes GetViewOffset() and GetZoomAmount() as public methods.
	const FVector2D ViewOffset = GraphPanel->GetViewOffset();
	const float     ZoomAmount = GraphPanel->GetZoomAmount();

	// Obtain the visible size of the graph panel in graph-space units.
	const FGeometry PanelGeometry  = GraphPanel->GetCachedGeometry();
	const FVector2D PanelLocalSize = PanelGeometry.GetLocalSize();

	// Visible area in graph space:
	//   The graph panel's top-left corner in graph space is ViewOffset.
	//   The visible extent in graph space is PanelLocalSize / ZoomAmount.
	const FVector2D ViewportGraphTL = ViewOffset;
	const FVector2D ViewportGraphBR = ViewOffset + (PanelLocalSize / ZoomAmount);

	// Transform both corners to minimap space.
	const FVector2D ViewportMinimapTL = TransformGraphToMinimap(ViewportGraphTL, MinimapSize);
	const FVector2D ViewportMinimapBR = TransformGraphToMinimap(ViewportGraphBR, MinimapSize);
	const FVector2D ViewportMinimapSize = ViewportMinimapBR - ViewportMinimapTL;

	// Only draw when the viewport rect is within the minimap bounds.
	if (ViewportMinimapSize.X <= 0.0f || ViewportMinimapSize.Y <= 0.0f)
	{
		return;
	}

	FLinearColor VpColor = ViewportIndicatorColor;
	VpColor.A *= Opacity;

	// Draw the viewport as an outlined rectangle (four line segments).
	FSlateDrawElement::MakeLines(
		OutDrawElements,
		LayerId,
		AllottedGeometry.ToPaintGeometry(),
		TArray<FVector2D>{
			ViewportMinimapTL,
			FVector2D(ViewportMinimapBR.X, ViewportMinimapTL.Y),
			ViewportMinimapBR,
			FVector2D(ViewportMinimapTL.X, ViewportMinimapBR.Y),
			ViewportMinimapTL
		},
		ESlateDrawEffect::None,
		VpColor,
		true,
		ViewportBorderThickness);

	// Draw a very faint filled interior so the viewport is legible on busy graphs.
	FLinearColor VpFill = ViewportIndicatorColor;
	VpFill.A *= Opacity * 0.08f;

	const FSlateBrush* FillBrush = FCoreStyle::Get().GetBrush("WhiteBrush");
	FSlateDrawElement::MakeBox(
		OutDrawElements,
		LayerId,
		AllottedGeometry.ToPaintGeometry(ViewportMinimapSize,
		                                 FSlateLayoutTransform(ViewportMinimapTL)),
		FillBrush,
		ESlateDrawEffect::None,
		VpFill);
}
