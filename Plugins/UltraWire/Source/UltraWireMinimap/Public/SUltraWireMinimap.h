// Copyright 2026 StraySpark. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "UltraWireTypes.h"

class SGraphPanel;
class UEdGraphNode;

/**
 * FMinimapNodeEntry
 *
 * Cached representation of a single graph node for minimap rendering.
 * Populated by CacheTopology() and consumed by OnPaint().
 */
struct FMinimapNodeEntry
{
	/** Node position in graph space (top-left corner). */
	FVector2D GraphPosition;

	/** Node size in graph space. */
	FVector2D GraphSize;

	/** Display color – derived from the node's GetNodeTitleColor(). */
	FLinearColor Color;

	/** True when this entry represents a comment box node. */
	bool bIsComment = false;
};

/**
 * FMinimapWireEntry
 *
 * Cached representation of a single wire for minimap rendering.
 * Stores only the start and end positions; intermediate control points
 * are not cached because they are expensive to recompute every frame and
 * visually indistinguishable at minimap scale.
 */
struct FMinimapWireEntry
{
	/** Pin output position in graph space. */
	FVector2D StartGraphPos;

	/** Pin input position in graph space. */
	FVector2D EndGraphPos;

	/** Wire color derived from the pin type (execution, bool, int, etc.). */
	FLinearColor Color;
};

/**
 * SUltraWireMinimap
 *
 * A Slate compound widget that renders a real-time bird's-eye overview of
 * the graph panel it is bound to.  It is intended to be added as a slot in
 * an SOverlay that already contains the SGraphPanel (see
 * FUltraWireMinimapModule::TryInjectMinimapIntoTab).
 *
 * Rendering
 * ---------
 *   - Background: a semi-transparent rounded rectangle.
 *   - Comment nodes: outlined rectangles in their comment colour.
 *   - Regular nodes: solid coloured rectangles sized proportionally.
 *   - Wires: thin single-pixel lines connecting node centres.
 *   - Viewport indicator: a white/grey outlined rect showing the currently
 *     visible portion of the graph.
 *
 * Interaction
 * -----------
 *   - Left-click: navigates the graph panel to the clicked graph coordinate.
 *   - The widget does not consume mouse events while dragging so that the
 *     parent overlay continues to receive input.
 *
 * Performance
 * -----------
 *   CacheTopology() is rate-limited to ~10 Hz via a timer accumulator in
 *   Tick().  OnPaint() always uses the last cached snapshot, ensuring that
 *   even on large graphs with thousands of nodes the paint call is fast.
 */
class ULTRAWIREMINIMAP_API SUltraWireMinimap : public SCompoundWidget
{
public:

	SLATE_BEGIN_ARGS(SUltraWireMinimap)
		: _GraphPanel(nullptr)
		{}

		/** The SGraphPanel that this minimap observes. */
		SLATE_ARGUMENT(TSharedPtr<SGraphPanel>, GraphPanel)

	SLATE_END_ARGS()

	/** Constructs and initialises the widget. */
	void Construct(const FArguments& InArgs);

	// SWidget overrides
	virtual int32 OnPaint(const FPaintArgs& Args,
	                      const FGeometry& AllottedGeometry,
	                      const FSlateRect& MyCullingRect,
	                      FSlateWindowElementList& OutDrawElements,
	                      int32 LayerId,
	                      const FWidgetStyle& InWidgetStyle,
	                      bool bParentEnabled) const override;

	virtual FVector2D ComputeDesiredSize(float LayoutScaleMultiplier) const override;

	virtual FReply OnMouseButtonDown(const FGeometry& MyGeometry,
	                                 const FPointerEvent& MouseEvent) override;

	virtual void Tick(const FGeometry& AllottedGeometry,
	                  const double InCurrentTime,
	                  const float InDeltaTime) override;

private:

	// ------------------------------------------------------------------
	// Internal helpers
	// ------------------------------------------------------------------

	/**
	 * Walks the graph panel's node collection and rebuilds CachedNodes and
	 * CachedWires.  This is rate-limited by TopologyUpdateAccumulator.
	 */
	void CacheTopology();

	/**
	 * Converts a position in graph space to a position in minimap widget-
	 * local space (pixels from the widget's top-left corner).
	 *
	 * @param GraphPos     The position to transform, in graph units.
	 * @param MinimapSize  The current allotted size of the minimap widget (pixels).
	 * @return             The position in minimap widget space.
	 */
	FVector2D TransformGraphToMinimap(FVector2D GraphPos,
	                                  FVector2D MinimapSize) const;

	/**
	 * Converts a position in minimap widget-local space back to graph space.
	 * Used by OnMouseButtonDown for click-to-navigate.
	 */
	FVector2D TransformMinimapToGraph(FVector2D MinimapPos,
	                                  FVector2D MinimapSize) const;

	/**
	 * Recomputes GraphBoundsMin / GraphBoundsMax from CachedNodes, adding a
	 * small margin.  Called at the end of CacheTopology().
	 */
	void RecalculateGraphBounds();

	/**
	 * Returns the currently-effective minimap size in pixels, read from
	 * UUltraWireSettings.
	 */
	float GetMinimapSizeFromSettings() const;

	/**
	 * Returns the currently-effective minimap opacity, read from
	 * UUltraWireSettings.
	 */
	float GetMinimapOpacityFromSettings() const;

	// ------------------------------------------------------------------
	// Rendering helpers
	// ------------------------------------------------------------------

	/** Paints the semi-transparent minimap background. */
	void PaintBackground(const FGeometry& AllottedGeometry,
	                     FSlateWindowElementList& OutDrawElements,
	                     int32 LayerId,
	                     float Opacity) const;

	/** Paints all cached node rectangles. */
	void PaintNodes(const FGeometry& AllottedGeometry,
	                FSlateWindowElementList& OutDrawElements,
	                int32 LayerId,
	                float Opacity) const;

	/** Paints all cached wire lines. */
	void PaintWires(const FGeometry& AllottedGeometry,
	                FSlateWindowElementList& OutDrawElements,
	                int32 LayerId,
	                float Opacity) const;

	/** Paints the viewport indicator rectangle. */
	void PaintViewport(const FGeometry& AllottedGeometry,
	                   FSlateWindowElementList& OutDrawElements,
	                   int32 LayerId,
	                   float Opacity) const;

	// ------------------------------------------------------------------
	// State
	// ------------------------------------------------------------------

	/** Weak pointer to the observed graph panel. */
	TWeakPtr<SGraphPanel> WeakGraphPanel;

	/** Cached node entries rebuilt at ~10 Hz. */
	TArray<FMinimapNodeEntry> CachedNodes;

	/** Cached wire entries rebuilt at ~10 Hz. */
	TArray<FMinimapWireEntry> CachedWires;

	/** Bounding box of all nodes in graph space, used for the mapping transform. */
	FVector2D GraphBoundsMin = FVector2D::ZeroVector;
	FVector2D GraphBoundsMax = FVector2D(1.0f, 1.0f);

	/**
	 * Accumulates delta time between Tick() calls.  CacheTopology() is called
	 * once this exceeds TopologyUpdateInterval.
	 */
	float TopologyUpdateAccumulator = 0.0f;

	/** Seconds between topology cache refreshes (~10 fps = 0.1 s). */
	static constexpr float TopologyUpdateInterval = 0.10f;

	/** Minimum padding around the bounding box in graph units. */
	static constexpr float BoundsPaddingGraphUnits = 100.0f;

	/** Minimum node rectangle size in minimap pixels so tiny nodes stay visible. */
	static constexpr float MinNodeRectSizePixels = 2.0f;
};
