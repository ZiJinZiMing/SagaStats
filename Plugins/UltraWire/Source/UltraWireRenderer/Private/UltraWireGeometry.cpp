// Copyright 2026 StraySpark. All Rights Reserved.

#include "UltraWireGeometry.h"
#include "Math/UnrealMathUtility.h"

// ---------------------------------------------------------------------------
// Internal constants
// ---------------------------------------------------------------------------

// Minimum segment length before we consider it degenerate and skip it.
static constexpr float MinSegLen = 0.5f;

// ---------------------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------------------

float FUltraWireGeometry::RibbonLateralOffset(int32 PinIndex, int32 TotalPins, float Spacing)
{
	if (TotalPins <= 1)
	{
		return 0.0f;
	}
	// Centre the bundle: indices run [0 .. TotalPins-1], centre is (TotalPins-1)/2.
	const float Centre = static_cast<float>(TotalPins - 1) * 0.5f;
	return (static_cast<float>(PinIndex) - Centre) * Spacing;
}

FVector2D FUltraWireGeometry::PerpendicularDir(FVector2D SegmentDir)
{
	// Rotate 90° counter-clockwise: (x,y) -> (-y, x)
	return FVector2D(-SegmentDir.Y, SegmentDir.X);
}

float FUltraWireGeometry::ComputePathLength(const TArray<FVector2D>& Points)
{
	float Len = 0.0f;
	for (int32 i = 1; i < Points.Num(); ++i)
	{
		Len += FVector2D::Distance(Points[i - 1], Points[i]);
	}
	return Len;
}

FVector2D FUltraWireGeometry::EvaluatePathAtT(
	const TArray<FVector2D>& Points,
	float T,
	int32* OutSegmentIndex)
{
	if (Points.Num() == 0)
	{
		return FVector2D::ZeroVector;
	}
	if (Points.Num() == 1)
	{
		if (OutSegmentIndex) { *OutSegmentIndex = 0; }
		return Points[0];
	}

	T = FMath::Clamp(T, 0.0f, 1.0f);

	const float TotalLen = ComputePathLength(Points);
	if (TotalLen < KINDA_SMALL_NUMBER)
	{
		if (OutSegmentIndex) { *OutSegmentIndex = 0; }
		return Points[0];
	}

	float TargetDist = T * TotalLen;
	float Accumulated = 0.0f;

	for (int32 i = 1; i < Points.Num(); ++i)
	{
		const float SegLen = FVector2D::Distance(Points[i - 1], Points[i]);
		if (SegLen < KINDA_SMALL_NUMBER)
		{
			continue;
		}
		if (Accumulated + SegLen >= TargetDist)
		{
			const float LocalT = (TargetDist - Accumulated) / SegLen;
			if (OutSegmentIndex) { *OutSegmentIndex = i - 1; }
			return FMath::Lerp(Points[i - 1], Points[i], LocalT);
		}
		Accumulated += SegLen;
	}

	if (OutSegmentIndex) { *OutSegmentIndex = Points.Num() - 2; }
	return Points.Last();
}

// ---------------------------------------------------------------------------
// ComputeManhattanPath
// ---------------------------------------------------------------------------

TArray<FVector2D> FUltraWireGeometry::ComputeManhattanPath(FVector2D Start, FVector2D End)
{
	TArray<FVector2D> Points;

	// Zero-length guard
	if (FVector2D::Distance(Start, End) < MinSegLen)
	{
		Points.Add(Start);
		Points.Add(End);
		return Points;
	}

	const float DeltaX = End.X - Start.X;
	const float DeltaY = End.Y - Start.Y;
	const bool bSameY = FMath::Abs(DeltaY) < MinSegLen;

	// ---- Forward connection (End is to the right of Start) ----
	if (DeltaX >= 0.0f)
	{
		if (bSameY)
		{
			// Pure horizontal – no bends needed
			Points.Add(Start);
			Points.Add(End);
			return Points;
		}

		// Three-segment L-route: right → down/up → right
		const float MidX = Start.X + DeltaX * 0.5f;

		Points.Add(Start);
		Points.Add(FVector2D(MidX, Start.Y));
		Points.Add(FVector2D(MidX, End.Y));
		Points.Add(End);
		return Points;
	}

	// ---- Backward connection (End is to the left of Start) ----
	// We loop back: right → down/up → left → down/up → right to End.
	// A vertical midpoint between Start and End is used for the lateral run.

	const float LoopX_Right = Start.X + BackwardLoopExtension;
	const float LoopX_Left  = End.X   - BackwardLoopExtension;

	const float MidY = Start.Y + DeltaY * 0.5f;

	Points.Add(Start);
	Points.Add(FVector2D(LoopX_Right, Start.Y));
	Points.Add(FVector2D(LoopX_Right, MidY));
	Points.Add(FVector2D(LoopX_Left,  MidY));
	Points.Add(FVector2D(LoopX_Left,  End.Y));
	Points.Add(End);

	return Points;
}

// ---------------------------------------------------------------------------
// ComputeSubwayPath
// ---------------------------------------------------------------------------

TArray<FVector2D> FUltraWireGeometry::ComputeSubwayPath(FVector2D Start, FVector2D End)
{
	TArray<FVector2D> Points;

	if (FVector2D::Distance(Start, End) < MinSegLen)
	{
		Points.Add(Start);
		Points.Add(End);
		return Points;
	}

	const float DeltaX = End.X - Start.X;
	const float DeltaY = End.Y - Start.Y;
	const bool bSameY = FMath::Abs(DeltaY) < MinSegLen;

	if (bSameY)
	{
		// Pure horizontal, no diagonal needed
		Points.Add(Start);
		Points.Add(End);
		return Points;
	}

	// The diagonal segment travels 45°, so it covers equal X and Y distance.
	// DiagLen is the absolute Y distance we need to bridge.
	const float AbsDY = FMath::Abs(DeltaY);

	if (DeltaX >= 0.0f)
	{
		// Forward connection:
		//   horizontal from Start to DiagonalStart
		//   45° diagonal for AbsDY units (X and Y both change by AbsDY)
		//   horizontal from DiagonalEnd to End
		const float HorizRun   = DeltaX - AbsDY;
		const float HalfHoriz  = HorizRun * 0.5f;

		if (HorizRun > 0.0f)
		{
			// There is room for horizontal segments on both sides
			const FVector2D DiagStart(Start.X + HalfHoriz, Start.Y);
			const FVector2D DiagEnd(DiagStart.X + AbsDY, End.Y);

			Points.Add(Start);
			Points.Add(DiagStart);
			Points.Add(DiagEnd);
			Points.Add(End);
		}
		else
		{
			// Not enough horizontal room; use pure diagonal (may be longer than 45°)
			// Place the diagonal as centred as possible.
			Points.Add(Start);
			Points.Add(End);
		}
	}
	else
	{
		// Backward connection – mirror the subway loop with diagonals
		const float LoopX_Right = Start.X + BackwardLoopExtension;
		const float LoopX_Left  = End.X   - BackwardLoopExtension;
		const float MidY        = Start.Y + DeltaY * 0.5f;
		const float HalfDY      = FMath::Abs(DeltaY) * 0.5f;

		const FVector2D Diag1Start(LoopX_Right, Start.Y);
		const FVector2D Diag1End  (LoopX_Right - HalfDY * FMath::Sign(LoopX_Right - LoopX_Left),
		                           MidY);

		const FVector2D Diag2Start(LoopX_Left + HalfDY * FMath::Sign(LoopX_Right - LoopX_Left),
		                           MidY);
		const FVector2D Diag2End  (LoopX_Left, End.Y);

		Points.Add(Start);
		Points.Add(Diag1Start);
		Points.Add(Diag1End);
		Points.Add(Diag2Start);
		Points.Add(Diag2End);
		Points.Add(End);
	}

	return Points;
}

// ---------------------------------------------------------------------------
// ComputeFreeformPath
// ---------------------------------------------------------------------------

TArray<FVector2D> FUltraWireGeometry::ComputeFreeformPath(
	FVector2D Start, FVector2D End, float AngleDeg)
{
	TArray<FVector2D> Points;

	if (FVector2D::Distance(Start, End) < MinSegLen)
	{
		Points.Add(Start);
		Points.Add(End);
		return Points;
	}

	// Clamp angle to prevent degenerate geometry
	AngleDeg = FMath::Clamp(AngleDeg, 5.0f, 85.0f);
	const float AngleRad  = FMath::DegreesToRadians(AngleDeg);
	const float TanAngle  = FMath::Tan(AngleRad);

	const float DeltaY = End.Y - Start.Y;
	const bool bSameY  = FMath::Abs(DeltaY) < MinSegLen;

	if (bSameY)
	{
		// Degenerate Y – just horizontal
		Points.Add(Start);
		Points.Add(End);
		return Points;
	}

	// The freeform path uses a symmetric design:
	//   horizontal exit from Start → diagonal at AngleDeg → horizontal entry to End
	//
	// The diagonal must cover DeltaY in Y, so it covers DeltaY/tan(angle) in X.
	const float DiagRunX = FMath::Abs(DeltaY) / TanAngle;

	// Distribute horizontal runs equally on both sides.
	const float TotalX     = End.X - Start.X;
	const float HorizEach  = (TotalX - DiagRunX) * 0.5f;

	if (HorizEach < 0.0f)
	{
		// Not enough horizontal room – fall back to a direct diagonal
		Points.Add(Start);
		Points.Add(End);
		return Points;
	}

	const FVector2D ExitPoint (Start.X + HorizEach, Start.Y);
	const FVector2D EntryPoint(End.X   - HorizEach, End.Y);

	Points.Add(Start);
	Points.Add(ExitPoint);
	Points.Add(EntryPoint);
	Points.Add(End);

	return Points;
}

// ---------------------------------------------------------------------------
// ApplyCornerRounding
// ---------------------------------------------------------------------------

TArray<FVector2D> FUltraWireGeometry::ApplyCornerRounding(
	const TArray<FVector2D>& Points,
	float Radius,
	EUltraWireCornerStyle Style,
	int32 ArcSubdivisions)
{
	if (Style == EUltraWireCornerStyle::Sharp || Points.Num() < 3 || Radius < KINDA_SMALL_NUMBER)
	{
		return Points;
	}

	ArcSubdivisions = FMath::Max(2, ArcSubdivisions);

	TArray<FVector2D> Result;
	Result.Reserve(Points.Num() * (ArcSubdivisions + 1));

	// Always include the first point unchanged
	Result.Add(Points[0]);

	for (int32 i = 1; i < Points.Num() - 1; ++i)
	{
		const FVector2D& Prev    = Points[i - 1];
		const FVector2D& Corner  = Points[i];
		const FVector2D& Next    = Points[i + 1];

		const FVector2D DirIn  = (Corner - Prev).GetSafeNormal();
		const FVector2D DirOut = (Next   - Corner).GetSafeNormal();

		// Clamp radius so it doesn't exceed half the shorter adjacent segment
		const float HalfIn  = FVector2D::Distance(Prev,   Corner) * 0.5f;
		const float HalfOut = FVector2D::Distance(Corner, Next)   * 0.5f;
		const float R       = FMath::Min(Radius, FMath::Min(HalfIn, HalfOut));

		if (R < KINDA_SMALL_NUMBER)
		{
			Result.Add(Corner);
			continue;
		}

		// Tangent points where the arc/chamfer begins and ends
		const FVector2D TangentIn  = Corner - DirIn  * R;
		const FVector2D TangentOut = Corner + DirOut * R;

		if (Style == EUltraWireCornerStyle::Chamfered)
		{
			Result.Add(TangentIn);
			Result.Add(TangentOut);
		}
		else // Rounded
		{
			// Find the arc centre.  For a right-angle corner the centre is at
			// Corner + Perp(DirIn)*R + DirOut*R, but we compute it generically
			// using the bisector so it works for non-right angles too.
			const FVector2D Bisector = (DirIn + DirOut).GetSafeNormal();
			const float     BisectorLen = (DirIn.Dot(DirOut) < -0.9999f)
				? R
				: R / FMath::Sin(FMath::Acos(FMath::Clamp(DirIn.Dot(DirOut), -1.0f, 1.0f)) * 0.5f);

			// Fall back to a chamfer for very shallow angles (near 180°)
			if (!FMath::IsFinite(BisectorLen) || BisectorLen > R * 10.0f)
			{
				Result.Add(TangentIn);
				Result.Add(TangentOut);
				continue;
			}

			const FVector2D ArcCentre = Corner - Bisector * BisectorLen;

			// Sweep angle
			const FVector2D ToTangentIn  = (TangentIn  - ArcCentre).GetSafeNormal();
			const FVector2D ToTangentOut = (TangentOut - ArcCentre).GetSafeNormal();

			float AngleStart = FMath::Atan2(ToTangentIn.Y,  ToTangentIn.X);
			float AngleEnd   = FMath::Atan2(ToTangentOut.Y, ToTangentOut.X);

			// Determine sweep direction (always take the short arc)
			float Sweep = AngleEnd - AngleStart;
			if (Sweep > PI)  { Sweep -= 2.0f * PI; }
			if (Sweep < -PI) { Sweep += 2.0f * PI; }

			// Emit arc subdivisions
			for (int32 s = 0; s <= ArcSubdivisions; ++s)
			{
				const float Frac = static_cast<float>(s) / static_cast<float>(ArcSubdivisions);
				const float Ang  = AngleStart + Sweep * Frac;
				Result.Add(ArcCentre + FVector2D(FMath::Cos(Ang), FMath::Sin(Ang)) * R);
			}
		}
	}

	// Always include the last point unchanged
	Result.Add(Points.Last());

	return Result;
}

// ---------------------------------------------------------------------------
// ComputeRibbonOffset
// ---------------------------------------------------------------------------

TArray<FVector2D> FUltraWireGeometry::ComputeRibbonOffset(
	const TArray<FVector2D>& Points,
	int32 PinIndex,
	int32 TotalPins,
	float Offset)
{
	if (Points.Num() < 2 || TotalPins <= 1 || Offset < KINDA_SMALL_NUMBER)
	{
		return Points;
	}

	const float LateralDist = RibbonLateralOffset(PinIndex, TotalPins, Offset);

	if (FMath::Abs(LateralDist) < KINDA_SMALL_NUMBER)
	{
		return Points;
	}

	TArray<FVector2D> Result;
	Result.Reserve(Points.Num());

	for (int32 i = 0; i < Points.Num(); ++i)
	{
		// Compute the average perpendicular at each vertex using adjacent segments.
		FVector2D Perp(0.0f, 0.0f);

		if (i > 0)
		{
			const FVector2D SegDir = (Points[i] - Points[i - 1]).GetSafeNormal();
			Perp += PerpendicularDir(SegDir);
		}
		if (i < Points.Num() - 1)
		{
			const FVector2D SegDir = (Points[i + 1] - Points[i]).GetSafeNormal();
			Perp += PerpendicularDir(SegDir);
		}

		Perp = Perp.GetSafeNormal();
		Result.Add(Points[i] + Perp * LateralDist);
	}

	return Result;
}
