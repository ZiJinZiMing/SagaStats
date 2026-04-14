// Copyright 2026 StraySpark. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UltraWireTypes.h"

/**
 * Pure static geometry engine for UltraWire.
 *
 * All methods produce a sequence of FVector2D waypoints that can be fed directly
 * into FSlateDrawElement::MakeLines().  The caller is responsible for any
 * zoom/offset transforms required before drawing.
 *
 * Coordinate convention: X increases to the right (output pin → input pin),
 * Y increases downward, consistent with Slate graph-panel space.
 */
class ULTRAWIRERENDERER_API FUltraWireGeometry
{
public:
	// -------------------------------------------------------------------------
	// Routing
	// -------------------------------------------------------------------------

	/**
	 * ComputeManhattanPath
	 *
	 * Routes the wire with axis-aligned (90°) segments only.
	 * The algorithm uses a three-segment layout:
	 *   1. Horizontal from Start to the midpoint X
	 *   2. Vertical from StartY to EndY
	 *   3. Horizontal from midpoint X to End
	 *
	 * When End.X < Start.X (backward connection) a detour is inserted so the
	 * wire loops back without overlapping the source node.
	 *
	 * @param Start   Output-pin attachment point in graph space.
	 * @param End     Input-pin attachment point in graph space.
	 * @return        Ordered waypoints including Start and End.
	 */
	static TArray<FVector2D> ComputeManhattanPath(FVector2D Start, FVector2D End);

	/**
	 * ComputeSubwayPath
	 *
	 * Routes using a combination of horizontal and 45° diagonal segments,
	 * similar to PCB subway routing.  The diagonal section bridges the Y-gap
	 * between the horizontal run from the start and the horizontal run into
	 * the end.
	 *
	 * @param Start   Output-pin attachment point.
	 * @param End     Input-pin attachment point.
	 * @return        Ordered waypoints including Start and End.
	 */
	static TArray<FVector2D> ComputeSubwayPath(FVector2D Start, FVector2D End);

	/**
	 * ComputeFreeformPath
	 *
	 * Routes with a user-defined exit angle (degrees) from the Start pin,
	 * followed by a straight diagonal, then a symmetric entry into End.
	 *
	 * @param Start   Output-pin attachment point.
	 * @param End     Input-pin attachment point.
	 * @param AngleDeg Exit angle in degrees measured from the +X axis.
	 *                 Clamped to [5, 85] to prevent degenerate geometry.
	 * @return        Ordered waypoints including Start and End.
	 */
	static TArray<FVector2D> ComputeFreeformPath(FVector2D Start, FVector2D End, float AngleDeg);

	// -------------------------------------------------------------------------
	// Corner rounding / chamfering
	// -------------------------------------------------------------------------

	/**
	 * ApplyCornerRounding
	 *
	 * Takes a polyline (including Start and End) and returns a new polyline
	 * with corners replaced by either arc approximations or chamfer cuts,
	 * depending on Style.  Sharp corners are returned unchanged.
	 *
	 * Arc corners are approximated with a configurable number of sub-segments
	 * (ArcSubdivisions).  Chamfered corners are a single diagonal cut.
	 *
	 * The radius is automatically clamped so it cannot exceed half the length
	 * of either adjacent segment, preventing overlapping arcs.
	 *
	 * @param Points         Input polyline waypoints.
	 * @param Radius         Desired corner radius in graph-space units.
	 * @param Style          Sharp / Rounded / Chamfered.
	 * @param ArcSubdivisions Number of line segments per arc (default 8).
	 * @return               New polyline with shaped corners.
	 */
	static TArray<FVector2D> ApplyCornerRounding(
		const TArray<FVector2D>& Points,
		float Radius,
		EUltraWireCornerStyle Style,
		int32 ArcSubdivisions = 8);

	// -------------------------------------------------------------------------
	// Ribbon stacking
	// -------------------------------------------------------------------------

	/**
	 * ComputeRibbonOffset
	 *
	 * Offsets an entire wire path perpendicularly to the local wire direction
	 * at each segment.  Used to visually stack multiple wires between the same
	 * pair of nodes as a parallel ribbon.
	 *
	 * @param Points     Base wire path (already routed).
	 * @param PinIndex   Zero-based index of this wire within the ribbon bundle.
	 * @param TotalPins  Total number of wires in the bundle.
	 * @param Offset     Per-wire spacing in graph-space units.
	 * @return           Offset copy of Points.
	 */
	static TArray<FVector2D> ComputeRibbonOffset(
		const TArray<FVector2D>& Points,
		int32 PinIndex,
		int32 TotalPins,
		float Offset);

	// -------------------------------------------------------------------------
	// Helpers (public so other systems can use them)
	// -------------------------------------------------------------------------

	/** Returns the total arc-length of a polyline. */
	static float ComputePathLength(const TArray<FVector2D>& Points);

	/**
	 * Evaluates a position along a polyline at normalized parameter T in [0,1].
	 * Returns the segment index via OutSegmentIndex for callers that need it.
	 */
	static FVector2D EvaluatePathAtT(
		const TArray<FVector2D>& Points,
		float T,
		int32* OutSegmentIndex = nullptr);

	/**
	 * Returns the outward perpendicular direction of a segment (rotated 90° CCW).
	 */
	static FVector2D PerpendicularDir(FVector2D SegmentDir);

private:
	/** Lateral separation from bundle center for a given wire index. */
	static float RibbonLateralOffset(int32 PinIndex, int32 TotalPins, float Spacing);

	/** Minimum horizontal extension used when routing backward connections. */
	static constexpr float BackwardLoopExtension = 40.0f;

	/** How far ahead of each pin the wire exits before turning. */
	static constexpr float PinExitLength = 20.0f;
};
