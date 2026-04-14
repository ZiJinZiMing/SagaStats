// Copyright 2026 StraySpark. All Rights Reserved.

#include "UltraWireRouteEngine.h"

#include "Containers/Array.h"
#include "Containers/Map.h"
#include "Containers/Set.h"
#include "Algo/Reverse.h"
#include "HAL/PlatformTime.h"
#include "Logging/LogMacros.h"
#include "Math/Box2D.h"
#include "Math/NumericLimits.h"
#include "Math/UnrealMathUtility.h"
#include "Misc/ScopeLock.h"

DEFINE_LOG_CATEGORY_STATIC(LogUltraWireRouter, Log, All);

// ---------------------------------------------------------------------------
// Static member definitions
// ---------------------------------------------------------------------------

TMap<uint32, FUltraWireRouteEngine::FCachedRoute> FUltraWireRouteEngine::RouteCache;

// ---------------------------------------------------------------------------
// A* node used internally during search
// ---------------------------------------------------------------------------

namespace
{
	struct FAStarNode
	{
		FRouteCell Cell;

		/** g: cost from the start cell to this cell. */
		float G = 0.0f;

		/** f = g + h: estimated total cost through this cell. */
		float F = 0.0f;

		// For the min-heap comparator (lowest F first).
		bool operator>(const FAStarNode& Other) const { return F > Other.F; }
	};

	/**
	 * Minimal binary min-heap (priority queue) for A* open list.
	 * TArray<FAStarNode> sorted by ascending F value.
	 */
	class FAStarHeap
	{
	public:
		void Push(const FAStarNode& Node)
		{
			Data.HeapPush(Node, [](const FAStarNode& A, const FAStarNode& B)
			{
				return A.F < B.F;
			});
		}

		FAStarNode Pop()
		{
			FAStarNode Top;
			Data.HeapPop(Top, [](const FAStarNode& A, const FAStarNode& B)
			{
				return A.F < B.F;
			});
			return Top;
		}

		bool IsEmpty() const { return Data.IsEmpty(); }

	private:
		TArray<FAStarNode> Data;
	};
} // anonymous namespace

// ---------------------------------------------------------------------------
// ComputeSmartRoute
// ---------------------------------------------------------------------------

TArray<FVector2D> FUltraWireRouteEngine::ComputeSmartRoute(
	FVector2D                    Start,
	FVector2D                    End,
	const TArray<FBox2D>&        NodeRects,
	float                        GridSize,
	float                        NodePadding,
	EUltraWireStyle              PreferredStyle)
{
	// Clamp parameters to sane ranges.
	const float ClampedGridSize    = FMath::Clamp(GridSize,    5.0f,  50.0f);
	const float ClampedNodePadding = FMath::Clamp(NodePadding, 0.0f, 100.0f);

	// ------------------------------------------------------------------
	// 1. Check the cache first.
	// ------------------------------------------------------------------
	const uint32 CacheKey = ComputeCacheKey(Start, End, NodeRects);

	if (const FCachedRoute* Cached = RouteCache.Find(CacheKey))
	{
		UE_LOG(LogUltraWireRouter, Verbose, TEXT("ComputeSmartRoute: cache hit (key=%u)."), CacheKey);
		return Cached->Path;
	}

	// ------------------------------------------------------------------
	// 2. Build the occupancy grid.
	// ------------------------------------------------------------------
	FRouteGrid Grid = BuildGrid(NodeRects, ClampedGridSize, ClampedNodePadding);

	if (Grid.Cols == 0 || Grid.Rows == 0)
	{
		UE_LOG(LogUltraWireRouter, Warning, TEXT("ComputeSmartRoute: degenerate grid, skipping A*."));
		return TArray<FVector2D>();
	}

	MarkOccupiedCells(Grid, NodeRects, ClampedNodePadding);

	// ------------------------------------------------------------------
	// 3. Map Start / End into grid cells.
	//    Ensure the endpoints themselves are never blocked – they are pin
	//    attachment points which may sit on the border of a node rect.
	// ------------------------------------------------------------------
	FRouteCell StartCell = WorldToGrid(Start, Grid);
	FRouteCell EndCell   = WorldToGrid(End,   Grid);

	// Force-unblock start and end so A* can depart from and arrive at them.
	Grid.Unblock(StartCell.Col, StartCell.Row);
	Grid.Unblock(EndCell.Col,   EndCell.Row);

	// ------------------------------------------------------------------
	// 4. Run A*.
	// ------------------------------------------------------------------
	const bool bAllowDiagonal = (PreferredStyle != EUltraWireStyle::Manhattan);

	TArray<FRouteCell> CellPath = FindPath(Grid, StartCell, EndCell,
	                                        bAllowDiagonal, AStarTimeoutMs);

	if (CellPath.IsEmpty())
	{
		// No path found or timeout – caller falls back to simple routing.
		UE_LOG(LogUltraWireRouter, Verbose,
		       TEXT("ComputeSmartRoute: A* returned no path (timeout or no-path)."));
		return TArray<FVector2D>();
	}

	// ------------------------------------------------------------------
	// 5. Convert cell path to world space and smooth it.
	// ------------------------------------------------------------------
	TArray<FVector2D> WorldPath = SmoothPath(CellPath, Grid, Start, End);

	// ------------------------------------------------------------------
	// 6. Cache the result.
	// ------------------------------------------------------------------
	FCachedRoute& NewEntry = RouteCache.Add(CacheKey);
	NewEntry.Path      = WorldPath;
	NewEntry.NodeRects = NodeRects;

	UE_LOG(LogUltraWireRouter, Verbose,
	       TEXT("ComputeSmartRoute: path cached (key=%u, %d waypoints)."),
	       CacheKey, WorldPath.Num());

	return WorldPath;
}

// ---------------------------------------------------------------------------
// Cache management
// ---------------------------------------------------------------------------

void FUltraWireRouteEngine::InvalidateCache()
{
	const int32 PreviousCount = RouteCache.Num();
	RouteCache.Reset();
	UE_LOG(LogUltraWireRouter, Log,
	       TEXT("InvalidateCache: cleared %d cached routes."), PreviousCount);
}

void FUltraWireRouteEngine::InvalidateCacheForNode(const FBox2D& NodeRect)
{
	// Expand the node rect by the typical maximum padding so we evict routes
	// that passed through the clearance zone as well.
	const FBox2D ExpandedRect = NodeRect.ExpandBy(50.0f);

	int32 Evicted = 0;
	for (auto It = RouteCache.CreateIterator(); It; ++It)
	{
		// Check if any waypoint of the cached path falls inside the expanded rect.
		bool bIntersects = false;
		for (const FVector2D& WP : It.Value().Path)
		{
			if (ExpandedRect.IsInside(WP))
			{
				bIntersects = true;
				break;
			}
		}

		// Also check if the original node rects snapshot contains a rect that
		// overlaps NodeRect – this catches routes whose intermediary cells passed
		// near the moved node even if no waypoint landed inside the expanded rect.
		if (!bIntersects)
		{
			for (const FBox2D& CachedRect : It.Value().NodeRects)
			{
				if (CachedRect.Intersect(ExpandedRect))
				{
					bIntersects = true;
					break;
				}
			}
		}

		if (bIntersects)
		{
			It.RemoveCurrent();
			++Evicted;
		}
	}

	UE_LOG(LogUltraWireRouter, Verbose,
	       TEXT("InvalidateCacheForNode: evicted %d routes."), Evicted);
}

// ---------------------------------------------------------------------------
// BuildGrid
// ---------------------------------------------------------------------------

FRouteGrid FUltraWireRouteEngine::BuildGrid(
	const TArray<FBox2D>& NodeRects,
	float                 CellSize,
	float                 Padding)
{
	FRouteGrid Grid;
	Grid.CellSize = FMath::Max(CellSize, 1.0f);

	if (NodeRects.IsEmpty())
	{
		// No nodes – create a small default grid so the caller still gets a result.
		Grid.WorldOrigin = FVector2D::ZeroVector;
		Grid.Cols        = 64;
		Grid.Rows        = 64;
		Grid.Cells.SetNumZeroed(Grid.Cols * Grid.Rows);
		return Grid;
	}

	// Compute the union bounding box of all node rects.
	FBox2D UnionBox(ForceInit);
	for (const FBox2D& Rect : NodeRects)
	{
		UnionBox += Rect;
	}

	// Expand by padding + border cells so routed wires have space on all sides.
	const float BorderWorld = (Padding + static_cast<float>(GridBorderCells) * CellSize);
	const FVector2D Expanded_Min = UnionBox.Min - FVector2D(BorderWorld, BorderWorld);
	const FVector2D Expanded_Max = UnionBox.Max + FVector2D(BorderWorld, BorderWorld);

	Grid.WorldOrigin = Expanded_Min;

	const FVector2D WorldExtent = Expanded_Max - Expanded_Min;
	Grid.Cols = FMath::Max(1, FMath::CeilToInt(WorldExtent.X / CellSize));
	Grid.Rows = FMath::Max(1, FMath::CeilToInt(WorldExtent.Y / CellSize));

	// Cap grid size to avoid pathological cases (very large graphs with tiny cells).
	const int32 MaxCells = 512;
	if (Grid.Cols > MaxCells || Grid.Rows > MaxCells)
	{
		UE_LOG(LogUltraWireRouter, Warning,
		       TEXT("BuildGrid: grid too large (%dx%d), capping to %dx%d. "
		            "Increase GridSize to improve performance."),
		       Grid.Cols, Grid.Rows, MaxCells, MaxCells);
		Grid.Cols = FMath::Min(Grid.Cols, MaxCells);
		Grid.Rows = FMath::Min(Grid.Rows, MaxCells);
	}

	Grid.Cells.SetNumZeroed(Grid.Cols * Grid.Rows);

	return Grid;
}

// ---------------------------------------------------------------------------
// MarkOccupiedCells
// ---------------------------------------------------------------------------

void FUltraWireRouteEngine::MarkOccupiedCells(
	FRouteGrid&           Grid,
	const TArray<FBox2D>& NodeRects,
	float                 Padding)
{
	for (const FBox2D& Rect : NodeRects)
	{
		// Expand the rect by the padding clearance.
		const FBox2D Expanded = Rect.ExpandBy(Padding);

		// Map the expanded rect corners into grid cells and block everything inside.
		const FRouteCell MinCell = WorldToGrid(Expanded.Min, Grid);
		const FRouteCell MaxCell = WorldToGrid(Expanded.Max, Grid);

		for (int32 Row = MinCell.Row; Row <= MaxCell.Row; ++Row)
		{
			for (int32 Col = MinCell.Col; Col <= MaxCell.Col; ++Col)
			{
				Grid.Block(Col, Row);
			}
		}
	}
}

// ---------------------------------------------------------------------------
// FindPath (A*)
// ---------------------------------------------------------------------------

TArray<FRouteCell> FUltraWireRouteEngine::FindPath(
	const FRouteGrid& Grid,
	FRouteCell        StartCell,
	FRouteCell        EndCell,
	bool              bAllowDiagonal,
	float             TimeoutMs)
{
	// If start == end, trivial path.
	if (StartCell == EndCell)
	{
		return { StartCell };
	}

	// If either endpoint is blocked we cannot route.
	// (Callers should have force-unblocked them, but be defensive.)
	if (Grid.IsBlocked(StartCell.Col, StartCell.Row) ||
	    Grid.IsBlocked(EndCell.Col,   EndCell.Row))
	{
		UE_LOG(LogUltraWireRouter, Warning,
		       TEXT("FindPath: start or end cell is blocked – cannot route."));
		return TArray<FRouteCell>();
	}

	const double StartTime = FPlatformTime::Seconds();
	const double TimeoutSec = TimeoutMs / 1000.0;

	// Heuristic function selected based on movement mode.
	auto Heuristic = bAllowDiagonal
		? &FUltraWireRouteEngine::HeuristicOctile
		: &FUltraWireRouteEngine::HeuristicManhattan;

	// A* state.
	TMap<FRouteCell, float>       GScore;   // best known cost from start
	TMap<FRouteCell, FRouteCell>  CameFrom; // path reconstruction map
	TSet<FRouteCell>              ClosedSet;

	GScore.Add(StartCell, 0.0f);

	FAStarHeap OpenHeap;
	FAStarNode StartNode;
	StartNode.Cell = StartCell;
	StartNode.G    = 0.0f;
	StartNode.F    = Heuristic(StartCell, EndCell);
	OpenHeap.Push(StartNode);

	// Movement directions.
	const int32 NumDirections = bAllowDiagonal ? 8 : 4;
	const int32 DCol[] = { 1,  0, -1,  0,   1,  1, -1, -1 };
	const int32 DRow[] = { 0,  1,  0, -1,   1, -1,  1, -1 };
	const float MoveCost[] = {
		CardinalMoveCost, CardinalMoveCost, CardinalMoveCost, CardinalMoveCost,
		DiagonalMoveCost, DiagonalMoveCost, DiagonalMoveCost, DiagonalMoveCost
	};

	while (!OpenHeap.IsEmpty())
	{
		// Timeout check – exit before we blow the frame budget.
		const double Elapsed = FPlatformTime::Seconds() - StartTime;
		if (Elapsed > TimeoutSec)
		{
			UE_LOG(LogUltraWireRouter, Warning,
			       TEXT("FindPath: A* timed out after %.1f ms (open list not empty)."),
			       Elapsed * 1000.0);
			return TArray<FRouteCell>();
		}

		FAStarNode Current = OpenHeap.Pop();

		if (Current.Cell == EndCell)
		{
			// Goal reached – reconstruct path.
			return ReconstructPath(CameFrom, Current.Cell);
		}

		if (ClosedSet.Contains(Current.Cell))
		{
			// Already processed via a cheaper path.
			continue;
		}
		ClosedSet.Add(Current.Cell);

		// Expand neighbours.
		for (int32 Dir = 0; Dir < NumDirections; ++Dir)
		{
			const FRouteCell Neighbour(
				Current.Cell.Col + DCol[Dir],
				Current.Cell.Row + DRow[Dir]);

			if (ClosedSet.Contains(Neighbour))
			{
				continue;
			}

			if (Grid.IsBlocked(Neighbour.Col, Neighbour.Row))
			{
				continue;
			}

			// For diagonal moves, verify neither orthogonal neighbour is blocked
			// (prevent clipping through corners of node rects).
			if (Dir >= 4)
			{
				const bool bColBlocked = Grid.IsBlocked(Current.Cell.Col + DCol[Dir], Current.Cell.Row);
				const bool bRowBlocked = Grid.IsBlocked(Current.Cell.Col, Current.Cell.Row + DRow[Dir]);
				if (bColBlocked || bRowBlocked)
				{
					continue;
				}
			}

			const float TentativeG = Current.G + MoveCost[Dir];
			const float* ExistingG = GScore.Find(Neighbour);

			if (ExistingG && TentativeG >= *ExistingG)
			{
				// Not a better path.
				continue;
			}

			GScore.FindOrAdd(Neighbour) = TentativeG;
			CameFrom.FindOrAdd(Neighbour) = Current.Cell;

			FAStarNode NeighbourNode;
			NeighbourNode.Cell = Neighbour;
			NeighbourNode.G    = TentativeG;
			NeighbourNode.F    = TentativeG + Heuristic(Neighbour, EndCell);
			OpenHeap.Push(NeighbourNode);
		}
	}

	// Open list exhausted with no path found.
	UE_LOG(LogUltraWireRouter, Verbose, TEXT("FindPath: no path found."));
	return TArray<FRouteCell>();
}

// ---------------------------------------------------------------------------
// Coordinate conversions
// ---------------------------------------------------------------------------

FVector2D FUltraWireRouteEngine::GridToWorld(const FRouteCell& Cell, const FRouteGrid& Grid)
{
	// Return the centre of the cell.
	return FVector2D(
		Grid.WorldOrigin.X + (static_cast<float>(Cell.Col) + 0.5f) * Grid.CellSize,
		Grid.WorldOrigin.Y + (static_cast<float>(Cell.Row) + 0.5f) * Grid.CellSize);
}

FRouteCell FUltraWireRouteEngine::WorldToGrid(FVector2D WorldPos, const FRouteGrid& Grid)
{
	const FVector2D LocalPos = WorldPos - Grid.WorldOrigin;
	const int32 Col = FMath::Clamp(
		FMath::FloorToInt(LocalPos.X / Grid.CellSize), 0, Grid.Cols - 1);
	const int32 Row = FMath::Clamp(
		FMath::FloorToInt(LocalPos.Y / Grid.CellSize), 0, Grid.Rows - 1);
	return FRouteCell(Col, Row);
}

// ---------------------------------------------------------------------------
// SmoothPath
// ---------------------------------------------------------------------------

TArray<FVector2D> FUltraWireRouteEngine::SmoothPath(
	const TArray<FRouteCell>& CellPath,
	const FRouteGrid&         Grid,
	FVector2D                 Start,
	FVector2D                 End)
{
	if (CellPath.IsEmpty())
	{
		return TArray<FVector2D>();
	}

	// Convert cells to world-space positions.
	TArray<FVector2D> WorldPoints;
	WorldPoints.Reserve(CellPath.Num() + 2);

	// Replace first and last cell centres with the exact pin positions.
	WorldPoints.Add(Start);

	for (int32 i = 1; i < CellPath.Num() - 1; ++i)
	{
		WorldPoints.Add(GridToWorld(CellPath[i], Grid));
	}

	WorldPoints.Add(End);

	// Collapse collinear segments: walk the polyline and remove interior points
	// that lie on the same direction vector as the preceding segment.
	TArray<FVector2D> Smoothed;
	Smoothed.Reserve(WorldPoints.Num());

	if (WorldPoints.Num() <= 2)
	{
		return WorldPoints;
	}

	Smoothed.Add(WorldPoints[0]);

	for (int32 i = 1; i < WorldPoints.Num() - 1; ++i)
	{
		const FVector2D PrevDir = (WorldPoints[i]     - WorldPoints[i - 1]).GetSafeNormal();
		const FVector2D NextDir = (WorldPoints[i + 1] - WorldPoints[i]).GetSafeNormal();

		// Keep the point only if the direction changes.
		const float DotProduct = FVector2D::DotProduct(PrevDir, NextDir);
		const bool bCollinear  = FMath::IsNearlyEqual(DotProduct, 1.0f, 0.01f);

		if (!bCollinear)
		{
			Smoothed.Add(WorldPoints[i]);
		}
	}

	Smoothed.Add(WorldPoints.Last());

	return Smoothed;
}

// ---------------------------------------------------------------------------
// Cache key
// ---------------------------------------------------------------------------

uint32 FUltraWireRouteEngine::ComputeCacheKey(
	FVector2D             Start,
	FVector2D             End,
	const TArray<FBox2D>& NodeRects)
{
	uint32 Hash = 0;

	// Hash the start and end positions, quantised to avoid float noise.
	// Round to the nearest integer unit so sub-pixel jitter doesn't bust cache.
	const int32 SX = FMath::RoundToInt(Start.X);
	const int32 SY = FMath::RoundToInt(Start.Y);
	const int32 EX = FMath::RoundToInt(End.X);
	const int32 EY = FMath::RoundToInt(End.Y);

	Hash = HashCombine(Hash, GetTypeHash(SX));
	Hash = HashCombine(Hash, GetTypeHash(SY));
	Hash = HashCombine(Hash, GetTypeHash(EX));
	Hash = HashCombine(Hash, GetTypeHash(EY));

	// Hash each node rect. Order matters here – we assume NodeRects is in a
	// stable order (same order as the graph panel queries them).
	for (const FBox2D& Rect : NodeRects)
	{
		const int32 MinX = FMath::RoundToInt(Rect.Min.X);
		const int32 MinY = FMath::RoundToInt(Rect.Min.Y);
		const int32 MaxX = FMath::RoundToInt(Rect.Max.X);
		const int32 MaxY = FMath::RoundToInt(Rect.Max.Y);

		Hash = HashCombine(Hash, GetTypeHash(MinX));
		Hash = HashCombine(Hash, GetTypeHash(MinY));
		Hash = HashCombine(Hash, GetTypeHash(MaxX));
		Hash = HashCombine(Hash, GetTypeHash(MaxY));
	}

	return Hash;
}

// ---------------------------------------------------------------------------
// Heuristics
// ---------------------------------------------------------------------------

float FUltraWireRouteEngine::HeuristicManhattan(FRouteCell A, FRouteCell B)
{
	return static_cast<float>(FMath::Abs(A.Col - B.Col) + FMath::Abs(A.Row - B.Row));
}

float FUltraWireRouteEngine::HeuristicOctile(FRouteCell A, FRouteCell B)
{
	const float DCol = static_cast<float>(FMath::Abs(A.Col - B.Col));
	const float DRow = static_cast<float>(FMath::Abs(A.Row - B.Row));
	// Octile distance: cardinals cost 1, diagonals cost sqrt(2).
	return CardinalMoveCost * (DCol + DRow) +
	       (DiagonalMoveCost - 2.0f * CardinalMoveCost) * FMath::Min(DCol, DRow);
}

// ---------------------------------------------------------------------------
// ReconstructPath
// ---------------------------------------------------------------------------

TArray<FRouteCell> FUltraWireRouteEngine::ReconstructPath(
	const TMap<FRouteCell, FRouteCell>& CameFrom,
	FRouteCell                          Current)
{
	TArray<FRouteCell> Path;
	Path.Add(Current);

	while (const FRouteCell* Prev = CameFrom.Find(Current))
	{
		Current = *Prev;
		Path.Add(Current);
	}

	// The path was built backwards from goal to start – reverse it.
	Algo::Reverse(Path);

	return Path;
}
