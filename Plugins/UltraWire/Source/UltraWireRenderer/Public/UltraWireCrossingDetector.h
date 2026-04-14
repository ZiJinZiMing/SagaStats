// Copyright 2026 StraySpark. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UltraWireTypes.h"
#include "Rendering/DrawElements.h"

/**
 * FUltraWireCrossing
 *
 * Represents a single intersection between two wire paths.
 * WireIndexA < WireIndexB is always guaranteed.
 */
struct ULTRAWIRERENDERER_API FUltraWireCrossing
{
	/** Position of the intersection in graph panel absolute space. */
	FVector2D Position;

	/** Index of the first wire (lower index) in the AllWirePaths array. */
	int32 WireIndexA = INDEX_NONE;

	/** Index of the second wire (higher index) in the AllWirePaths array. */
	int32 WireIndexB = INDEX_NONE;
};

/**
 * FUltraWireCrossingDetector
 *
 * Stateless helper that detects all pair-wise segment intersections between a
 * set of wire paths and renders crossing symbols at each intersection.
 *
 * Algorithm:
 *   O(W^2 * S^2) where W = number of wires, S = average segments per wire.
 *   For typical Blueprint graphs (dozens of wires, ~3 segments each) this is
 *   fast enough to run every frame.  A spatial hash or bounding-box pre-cull
 *   can be added here if profiling shows it is needed.
 *
 * Coordinate convention:
 *   All positions are in graph panel absolute (screen) space, matching the
 *   coordinates passed to FConnectionDrawingPolicy::DrawConnection.
 *   Drawing uses FPaintGeometry(Position, Size, Scale) directly.
 */
class ULTRAWIRERENDERER_API FUltraWireCrossingDetector
{
public:
	/**
	 * DetectCrossings
	 *
	 * Finds every point where any segment of wire A intersects any segment of
	 * wire B (A != B).  Co-linear or touching-endpoint cases are ignored since
	 * they do not produce a visual crossing.
	 *
	 * @param AllWirePaths  One entry per drawn wire; each entry is the ordered
	 *                      waypoints of that wire in panel-absolute space.
	 * @return              Array of crossing records, one per unique intersection.
	 */
	static TArray<FUltraWireCrossing> DetectCrossings(
		const TArray<TArray<FVector2D>>& AllWirePaths);

	/**
	 * DrawCrossingSymbol
	 *
	 * Draws a single crossing indicator at Position using the chosen Style.
	 *
	 *   Gap    – draws nothing; the caller splits the lower wire at this point.
	 *   Arc    – a small semicircular arc that jumps over the crossing.
	 *   Circle – a filled circle indicating a solder joint (wires connected).
	 *
	 * @param DrawElements   Slate draw list to append to.
	 * @param LayerID        Slate layer to draw on.
	 * @param ZoomFactor     Current zoom factor for scaling the symbol.
	 * @param Position       Centre of the crossing symbol in panel-absolute space.
	 * @param Style          Visual style of the crossing indicator.
	 * @param Size           Diameter of the symbol in graph-space units
	 *                       (before zoom scaling).
	 * @param Color          Colour of the symbol.
	 */
	static void DrawCrossingSymbol(
		FSlateWindowElementList& DrawElements,
		int32 LayerID,
		float ZoomFactor,
		FVector2D Position,
		EUltraWireCrossingStyle Style,
		float Size,
		const FLinearColor& Color);

private:
	/**
	 * Tests whether segment AB intersects segment CD.
	 * Returns true and sets OutIntersection to the intersection point.
	 * Returns false for parallel, co-linear, or endpoint-only touches.
	 */
	static bool SegmentsIntersect(
		FVector2D A, FVector2D B,
		FVector2D C, FVector2D D,
		FVector2D& OutIntersection);

	/** Number of line segments used to approximate an arc crossing. */
	static constexpr int32 ArcSegments = 10;
};
