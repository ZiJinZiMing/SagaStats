// Copyright 2026 StraySpark. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Rendering/DrawElements.h"

/**
 * FUltraWireBubbleSystem
 *
 * Stateless helper that computes and draws animated "flow bubbles" along a
 * wire path.  All state (time, speed, spacing) is supplied by the caller so
 * there is no per-wire object that needs to be ticked or managed.
 *
 * Coordinate convention:
 *   WirePath and Positions are in graph panel absolute (screen) space,
 *   matching the coordinates passed to FConnectionDrawingPolicy::DrawConnection.
 *   Drawing uses FPaintGeometry(Position, Size, Scale) directly to avoid
 *   requiring a parent FGeometry reference.
 *
 * Usage in DrawConnection():
 *   double CurrentTime = FSlateApplication::Get().GetCurrentTime();
 *   auto Positions = FUltraWireBubbleSystem::ComputeBubblePositions(
 *       WirePath, CurrentTime, Profile.BubbleSpeed, Profile.BubbleSpacing);
 *   FUltraWireBubbleSystem::DrawBubbles(
 *       DrawElementsList, LayerID, ZoomFactor, Positions,
 *       Profile.BubbleSize, WireColor);
 */
class ULTRAWIRERENDERER_API FUltraWireBubbleSystem
{
public:
	/**
	 * ComputeBubblePositions
	 *
	 * Returns a list of 2-D positions, evenly spaced along the wire path,
	 * animated by Time.  Bubbles wrap around when they reach the end of the
	 * wire so there are always the same number of them visible.
	 *
	 * @param WirePath   Ordered waypoints of the wire in panel-absolute space.
	 * @param Time       Current time in seconds (monotonically increasing).
	 * @param Speed      Speed of bubbles in panel-space units per second.
	 * @param Spacing    Distance between consecutive bubbles in panel-space units.
	 * @return           Positions of bubbles ready to draw.
	 */
	static TArray<FVector2D> ComputeBubblePositions(
		const TArray<FVector2D>& WirePath,
		double Time,
		float Speed,
		float Spacing);

	/**
	 * DrawBubbles
	 *
	 * Renders each bubble position as a filled circle using MakeBox with the
	 * engine white brush.  The circle is centred on the position.
	 *
	 * @param DrawElements   Slate draw list to append to.
	 * @param LayerID        Slate layer ID (use ArrowLayerID from the policy).
	 * @param ZoomFactor     Current zoom factor; scales the diameter.
	 * @param Positions      Bubble centres in panel-absolute space.
	 * @param Size           Diameter of each bubble in graph-space units
	 *                       (before zoom scaling).
	 * @param Color          Linear colour (including alpha) of the bubbles.
	 */
	static void DrawBubbles(
		FSlateWindowElementList& DrawElements,
		int32 LayerID,
		float ZoomFactor,
		const TArray<FVector2D>& Positions,
		float Size,
		const FLinearColor& Color);
};
