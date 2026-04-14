// Copyright 2026 StraySpark. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Containers/Array.h"
#include "Containers/Map.h"
#include "Math/Vector2D.h"
#include "UltraWireTypes.h"

// ---------------------------------------------------------------------------
// FRouteGrid
// ---------------------------------------------------------------------------

/**
 * FRouteGrid
 *
 * Discretised occupancy grid used by the A* pathfinder.
 *
 * Each cell is either free (false) or blocked (true).  The grid covers the
 * bounding box of all node rectangles plus a configurable padding margin.
 *
 * Coordinate system:
 *   Column (X) increases to the right.
 *   Row    (Y) increases downward.
 *   Cell (0,0) maps to the top-left corner of the grid's world-space origin.
 */
struct ULTRAWIRERENDERER_API FRouteGrid
{
	/** Width in cells. */
	int32 Cols = 0;

	/** Height in cells. */
	int32 Rows = 0;

	/** World-space position of cell (0,0)'s top-left corner. */
	FVector2D WorldOrigin = FVector2D::ZeroVector;

	/** Side length of each square cell, in world-space units. */
	float CellSize = 20.0f;

	/**
	 * Flat occupancy array, row-major.
	 * Index = Row * Cols + Col.
	 * true  = cell is blocked (inside a node or padding region).
	 * false = cell is free.
	 */
	TArray<bool> Cells;

	// -------------------------------------------------------------------------
	// Helpers
	// -------------------------------------------------------------------------

	/** Returns true if (Col, Row) is within the grid bounds. */
	FORCEINLINE bool IsInBounds(int32 Col, int32 Row) const
	{
		return Col >= 0 && Col < Cols && Row >= 0 && Row < Rows;
	}

	/** Returns the occupancy of cell (Col, Row).  Out-of-bounds cells are blocked. */
	FORCEINLINE bool IsBlocked(int32 Col, int32 Row) const
	{
		if (!IsInBounds(Col, Row)) return true;
		return Cells[Row * Cols + Col];
	}

	/** Marks cell (Col, Row) as blocked.  Silently ignores out-of-bounds. */
	FORCEINLINE void Block(int32 Col, int32 Row)
	{
		if (IsInBounds(Col, Row))
		{
			Cells[Row * Cols + Col] = true;
		}
	}

	/** Marks cell (Col, Row) as free.  Silently ignores out-of-bounds. */
	FORCEINLINE void Unblock(int32 Col, int32 Row)
	{
		if (IsInBounds(Col, Row))
		{
			Cells[Row * Cols + Col] = false;
		}
	}
};

// ---------------------------------------------------------------------------
// FRouteCell
// ---------------------------------------------------------------------------

/** Lightweight (Col, Row) cell coordinate. */
struct FRouteCell
{
	int32 Col = 0;
	int32 Row = 0;

	FRouteCell() = default;
	FRouteCell(int32 InCol, int32 InRow) : Col(InCol), Row(InRow) {}

	FORCEINLINE bool operator==(const FRouteCell& Other) const
	{
		return Col == Other.Col && Row == Other.Row;
	}

	FORCEINLINE bool operator!=(const FRouteCell& Other) const
	{
		return !(*this == Other);
	}
};

FORCEINLINE uint32 GetTypeHash(const FRouteCell& Cell)
{
	return HashCombine(GetTypeHash(Cell.Col), GetTypeHash(Cell.Row));
}

// ---------------------------------------------------------------------------
// FUltraWireRouteEngine
// ---------------------------------------------------------------------------

/**
 * FUltraWireRouteEngine
 *
 * Static-method utility class that implements A* smart routing for UltraWire
 * wires.  All state is either passed in as parameters or stored in the thread-
 * local (editor-only) route cache managed by the private static members below.
 *
 * Usage
 * -----
 *   // Build the list of node bounding boxes in graph space.
 *   TArray<FBox2D> NodeRects = ...;
 *
 *   // Compute a routed path from one pin to another.
 *   TArray<FVector2D> Path = FUltraWireRouteEngine::ComputeSmartRoute(
 *       StartPin, EndPin, NodeRects, GridSize, Padding, EUltraWireStyle::Manhattan);
 *
 *   // Pass the result to FUltraWireGeometry::ApplyCornerRounding() and then
 *   // to FSlateDrawElement::MakeLines().
 *
 * Performance contract
 * --------------------
 *   ComputeSmartRoute() will return an empty array (triggering caller fallback
 *   to simple routing) if A* has not found a path within 50 ms.  The caller
 *   must handle an empty return without asserting.
 *
 * Cache invalidation
 * ------------------
 *   The route cache is keyed by a hash of (Start, End, NodeRects).
 *   Call InvalidateCache() when the graph layout changes significantly.
 *   Call InvalidateCacheForNode() when a single node moves to surgically
 *   evict only the routes that passed through or near that node.
 */
class ULTRAWIRERENDERER_API FUltraWireRouteEngine
{
public:
	// -------------------------------------------------------------------------
	// Primary API
	// -------------------------------------------------------------------------

	/**
	 * ComputeSmartRoute
	 *
	 * Plans a wire path from Start to End that avoids the supplied node
	 * bounding rectangles.
	 *
	 * Algorithm overview:
	 *   1. Build an FRouteGrid from the union of all NodeRects, extended by
	 *      a padding margin on each side.
	 *   2. Mark all cells that overlap any NodeRect (expanded by Padding) as
	 *      blocked.
	 *   3. Run A* from WorldToGrid(Start) to WorldToGrid(End).
	 *      - Manhattan heuristic for EUltraWireStyle::Manhattan.
	 *      - Octile (diagonal) heuristic for all other styles.
	 *   4. Convert the cell path back to world space with GridToWorld().
	 *   5. Run SmoothPath() to collapse collinear segments.
	 *   6. Return the result (may be empty on timeout or no-path).
	 *
	 * @param Start          Output-pin attachment point in graph space.
	 * @param End            Input-pin attachment point in graph space.
	 * @param NodeRects      Bounding boxes of all nodes in the graph.
	 * @param GridSize       Cell side length in world-space units (5-50).
	 * @param NodePadding    Extra clearance around node rects in world-space units.
	 * @param PreferredStyle Wire style – affects the A* movement heuristic.
	 * @return               Ordered waypoints from Start to End, or empty on failure.
	 */
	static TArray<FVector2D> ComputeSmartRoute(
		FVector2D                    Start,
		FVector2D                    End,
		const TArray<FBox2D>&        NodeRects,
		float                        GridSize,
		float                        NodePadding,
		EUltraWireStyle              PreferredStyle);

	// -------------------------------------------------------------------------
	// Cache management
	// -------------------------------------------------------------------------

	/**
	 * Clears the entire route cache.
	 * Call after bulk graph operations (e.g. paste, delete all).
	 */
	static void InvalidateCache();

	/**
	 * Surgically removes cache entries whose route passed through or near
	 * the supplied node bounding box.  Much cheaper than a full invalidation.
	 *
	 * @param NodeRect  The bounding box of the moved / resized node.
	 */
	static void InvalidateCacheForNode(const FBox2D& NodeRect);

	// -------------------------------------------------------------------------
	// Sub-system helpers (public for unit-testing)
	// -------------------------------------------------------------------------

	/**
	 * Allocates and initialises an FRouteGrid that covers the union of all
	 * NodeRects, extended by (Padding + one extra cell margin) on all sides.
	 *
	 * @param NodeRects  Bounding boxes used to determine grid extents.
	 * @param CellSize   World-space cell size.
	 * @param Padding    Extra clearance to add around each node rect.
	 * @return           Initialised grid (all cells free).
	 */
	static FRouteGrid BuildGrid(
		const TArray<FBox2D>& NodeRects,
		float                 CellSize,
		float                 Padding);

	/**
	 * Marks all cells that overlap any of the supplied node rects (each
	 * expanded by Padding) as blocked.
	 *
	 * @param Grid      Grid to modify.
	 * @param NodeRects Bounding boxes to rasterise.
	 * @param Padding   Extra clearance added to each rect before rasterising.
	 */
	static void MarkOccupiedCells(
		FRouteGrid&           Grid,
		const TArray<FBox2D>& NodeRects,
		float                 Padding);

	/**
	 * A* pathfinder.
	 *
	 * @param Grid         Occupancy grid.
	 * @param StartCell    Source cell.
	 * @param EndCell      Destination cell.
	 * @param bAllowDiagonal If true, 8-directional movement is permitted.
	 * @param TimeoutMs    Maximum wall-clock milliseconds before aborting.
	 * @return             Ordered cell path from StartCell to EndCell,
	 *                     or empty if no path exists or timeout was reached.
	 */
	static TArray<FRouteCell> FindPath(
		const FRouteGrid& Grid,
		FRouteCell        StartCell,
		FRouteCell        EndCell,
		bool              bAllowDiagonal,
		float             TimeoutMs = 50.0f);

	/**
	 * Converts a grid cell to its world-space centre position.
	 *
	 * @param Cell     Grid cell.
	 * @param Grid     The grid that owns the cell.
	 * @return         World-space centre of the cell.
	 */
	static FVector2D GridToWorld(const FRouteCell& Cell, const FRouteGrid& Grid);

	/**
	 * Converts a world-space position to the nearest grid cell.
	 * The result is clamped to the grid bounds.
	 *
	 * @param WorldPos World-space position.
	 * @param Grid     The grid to map into.
	 * @return         Nearest valid cell.
	 */
	static FRouteCell WorldToGrid(FVector2D WorldPos, const FRouteGrid& Grid);

	/**
	 * Converts a raw cell path into a world-space polyline and collapses
	 * adjacent collinear segments into single segments.
	 *
	 * @param CellPath  Ordered cell sequence from A*.
	 * @param Grid      Grid used for coordinate conversion.
	 * @param Start     Exact world-space start (replaces first cell centre).
	 * @param End       Exact world-space end (replaces last cell centre).
	 * @return          Smoothed world-space polyline.
	 */
	static TArray<FVector2D> SmoothPath(
		const TArray<FRouteCell>& CellPath,
		const FRouteGrid&         Grid,
		FVector2D                 Start,
		FVector2D                 End);

	// -------------------------------------------------------------------------
	// Cache key generation
	// -------------------------------------------------------------------------

	/**
	 * Computes a 32-bit hash that uniquely (with high probability) identifies a
	 * routing problem defined by (Start, End, NodeRects).
	 *
	 * @param Start     Route source.
	 * @param End       Route destination.
	 * @param NodeRects Current set of node bounding boxes.
	 * @return          Combined hash value.
	 */
	static uint32 ComputeCacheKey(
		FVector2D             Start,
		FVector2D             End,
		const TArray<FBox2D>& NodeRects);

private:
	// -------------------------------------------------------------------------
	// A* internals
	// -------------------------------------------------------------------------

	/** Manhattan distance heuristic (4-directional movement). */
	static float HeuristicManhattan(FRouteCell A, FRouteCell B);

	/** Octile distance heuristic (8-directional movement). */
	static float HeuristicOctile(FRouteCell A, FRouteCell B);

	/** Reconstructs the path from the A* came-from map. */
	static TArray<FRouteCell> ReconstructPath(
		const TMap<FRouteCell, FRouteCell>& CameFrom,
		FRouteCell                          Current);

	// -------------------------------------------------------------------------
	// Route cache
	// -------------------------------------------------------------------------

	/**
	 * TMap<CacheKey, CachedPath>
	 *
	 * Key:   uint32 hash from ComputeCacheKey().
	 * Value: Cached world-space waypoint array.
	 *
	 * We also store the contributing NodeRects so InvalidateCacheForNode()
	 * can evict selectively without recomputing hashes.
	 */
	struct FCachedRoute
	{
		TArray<FVector2D> Path;
		TArray<FBox2D>    NodeRects; // snapshot of rects when this was computed
	};

	/** The route cache.  Accessed only from the editor (game thread). */
	static TMap<uint32, FCachedRoute> RouteCache;

	// -------------------------------------------------------------------------
	// Constants
	// -------------------------------------------------------------------------

	/** Hard timeout (ms) after which A* aborts and returns an empty path. */
	static constexpr float AStarTimeoutMs = 50.0f;

	/** Extra cell border added around the node-rect union when sizing the grid. */
	static constexpr int32 GridBorderCells = 3;

	/** Movement costs for the A* open-list priority queue. */
	static constexpr float CardinalMoveCost  = 1.0f;
	static constexpr float DiagonalMoveCost  = 1.41421356f; // sqrt(2)
};
