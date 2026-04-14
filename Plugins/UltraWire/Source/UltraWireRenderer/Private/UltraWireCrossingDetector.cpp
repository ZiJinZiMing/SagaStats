// Copyright 2026 StraySpark. All Rights Reserved.

#include "UltraWireCrossingDetector.h"
#include "Styling/CoreStyle.h"
#include "Layout/PaintGeometry.h"

// ---------------------------------------------------------------------------
// SegmentsIntersect
// ---------------------------------------------------------------------------

bool FUltraWireCrossingDetector::SegmentsIntersect(
	FVector2D A, FVector2D B,
	FVector2D C, FVector2D D,
	FVector2D& OutIntersection)
{
	// Parametric intersection:
	// Segment 1: P = A + t*(B-A),  t in [0,1]
	// Segment 2: Q = C + u*(D-C),  u in [0,1]

	const FVector2D r = B - A;
	const FVector2D s = D - C;

	const double rCrossS = r.X * s.Y - r.Y * s.X;

	// Parallel (or co-linear) – no unique intersection
	if (FMath::Abs(rCrossS) < KINDA_SMALL_NUMBER)
	{
		return false;
	}

	const FVector2D QminusP = C - A;
	const double t = (QminusP.X * s.Y - QminusP.Y * s.X) / rCrossS;
	const double u = (QminusP.X * r.Y - QminusP.Y * r.X) / rCrossS;

	// Strict interior intersection: exclude endpoint touches.
	static constexpr double Eps = 1e-4;
	if (t > Eps && t < 1.0 - Eps && u > Eps && u < 1.0 - Eps)
	{
		OutIntersection = A + r * static_cast<float>(t);
		return true;
	}

	return false;
}

// ---------------------------------------------------------------------------
// DetectCrossings
// ---------------------------------------------------------------------------

TArray<FUltraWireCrossing> FUltraWireCrossingDetector::DetectCrossings(
	const TArray<TArray<FVector2D>>& AllWirePaths)
{
	TArray<FUltraWireCrossing> Result;

	const int32 NumWires = AllWirePaths.Num();
	if (NumWires < 2)
	{
		return Result;
	}

	for (int32 WireA = 0; WireA < NumWires - 1; ++WireA)
	{
		const TArray<FVector2D>& PathA = AllWirePaths[WireA];
		if (PathA.Num() < 2) { continue; }

		// Compute bounding box for wire A
		FVector2D BoxAMin = PathA[0], BoxAMax = PathA[0];
		for (const FVector2D& P : PathA)
		{
			BoxAMin.X = FMath::Min(BoxAMin.X, P.X); BoxAMin.Y = FMath::Min(BoxAMin.Y, P.Y);
			BoxAMax.X = FMath::Max(BoxAMax.X, P.X); BoxAMax.Y = FMath::Max(BoxAMax.Y, P.Y);
		}

		for (int32 WireB = WireA + 1; WireB < NumWires; ++WireB)
		{
			const TArray<FVector2D>& PathB = AllWirePaths[WireB];
			if (PathB.Num() < 2) { continue; }

			// Bounding-box pre-cull for the wire pair.
			FVector2D BoxBMin = PathB[0], BoxBMax = PathB[0];
			for (const FVector2D& P : PathB)
			{
				BoxBMin.X = FMath::Min(BoxBMin.X, P.X); BoxBMin.Y = FMath::Min(BoxBMin.Y, P.Y);
				BoxBMax.X = FMath::Max(BoxBMax.X, P.X); BoxBMax.Y = FMath::Max(BoxBMax.Y, P.Y);
			}

			// Axis-aligned bounding box overlap test.
			if (BoxAMax.X < BoxBMin.X || BoxBMax.X < BoxAMin.X ||
				BoxAMax.Y < BoxBMin.Y || BoxBMax.Y < BoxAMin.Y)
			{
				continue;
			}

			// Segment-level intersection tests.
			for (int32 SegA = 0; SegA < PathA.Num() - 1; ++SegA)
			{
				for (int32 SegB = 0; SegB < PathB.Num() - 1; ++SegB)
				{
					FVector2D Intersection;
					if (SegmentsIntersect(
						PathA[SegA], PathA[SegA + 1],
						PathB[SegB], PathB[SegB + 1],
						Intersection))
					{
						FUltraWireCrossing Crossing;
						Crossing.Position   = Intersection;
						Crossing.WireIndexA = WireA;
						Crossing.WireIndexB = WireB;
						Result.Add(Crossing);
					}
				}
			}
		}
	}

	return Result;
}

// ---------------------------------------------------------------------------
// DrawCrossingSymbol
// ---------------------------------------------------------------------------

void FUltraWireCrossingDetector::DrawCrossingSymbol(
	FSlateWindowElementList& DrawElements,
	int32 LayerID,
	float ZoomFactor,
	FVector2D Position,
	EUltraWireCrossingStyle Style,
	float Size,
	const FLinearColor& Color)
{
	if (Style == EUltraWireCrossingStyle::None || Size < KINDA_SMALL_NUMBER)
	{
		return;
	}

	const float ScaledRadius = Size * 0.5f * ZoomFactor;
	const uint32 Layer = static_cast<uint32>(LayerID);

	switch (Style)
	{
	case EUltraWireCrossingStyle::Gap:
		// Gap is handled by the drawing policy splitting the line at the
		// crossing; no separate symbol is drawn here.
		break;

	case EUltraWireCrossingStyle::Arc:
	{
		// Semicircular arc (upward bump) spanning 180° left-to-right.
		TArray<FVector2D> ArcPoints;
		ArcPoints.Reserve(ArcSegments + 1);

		for (int32 s = 0; s <= ArcSegments; ++s)
		{
			const float Frac = static_cast<float>(s) / static_cast<float>(ArcSegments);
			// Sweep from PI (left) to 0 (right) for an upward bump.
			const float Ang = PI * (1.0f - Frac);
			ArcPoints.Add(Position + FVector2D(
				FMath::Cos(Ang) * ScaledRadius,
				FMath::Sin(Ang) * ScaledRadius));
		}

		FSlateDrawElement::MakeLines(
			DrawElements,
			Layer,
			FPaintGeometry(),
			ArcPoints,
			ESlateDrawEffect::None,
			Color,
			/*bAntialias=*/true,
			/*Thickness=*/1.5f);
		break;
	}

	case EUltraWireCrossingStyle::Circle:
	{
		const FSlateBrush* CircleBrush = FCoreStyle::Get().GetBrush("WhiteBrush");
		if (CircleBrush)
		{
			const float Diameter = ScaledRadius * 2.0f;
			FSlateDrawElement::MakeBox(
				DrawElements,
				Layer,
				FPaintGeometry(
					FVector2D(Position.X - ScaledRadius, Position.Y - ScaledRadius),
					FVector2D(Diameter, Diameter),
					ZoomFactor),
				CircleBrush,
				ESlateDrawEffect::None,
				Color);
		}
		break;
	}

	default:
		break;
	}
}
