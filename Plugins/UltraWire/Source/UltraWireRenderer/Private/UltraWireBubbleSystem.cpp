// Copyright 2026 StraySpark. All Rights Reserved.

#include "UltraWireBubbleSystem.h"
#include "UltraWireGeometry.h"
#include "Styling/CoreStyle.h"
#include "Rendering/DrawElements.h"
#include "Layout/PaintGeometry.h"

// ---------------------------------------------------------------------------
// ComputeBubblePositions
// ---------------------------------------------------------------------------

TArray<FVector2D> FUltraWireBubbleSystem::ComputeBubblePositions(
	const TArray<FVector2D>& WirePath,
	double Time,
	float Speed,
	float Spacing)
{
	TArray<FVector2D> Result;

	if (WirePath.Num() < 2 || Spacing < KINDA_SMALL_NUMBER)
	{
		return Result;
	}

	const float PathLength = FUltraWireGeometry::ComputePathLength(WirePath);
	if (PathLength < KINDA_SMALL_NUMBER)
	{
		return Result;
	}

	// How far along the wire the "head" of the bubble train is at this moment.
	const float HeadDist = FMath::Fmod(
		static_cast<float>(Time) * Speed,
		PathLength + Spacing); // +Spacing so bubbles fade in/out cleanly

	// Walk backwards from HeadDist, placing a bubble every Spacing units.
	float BubbleDist = HeadDist;
	while (BubbleDist >= 0.0f)
	{
		const float T = BubbleDist / PathLength;
		if (T <= 1.0f)
		{
			Result.Add(FUltraWireGeometry::EvaluatePathAtT(WirePath, T));
		}
		BubbleDist -= Spacing;
	}

	return Result;
}

// ---------------------------------------------------------------------------
// DrawBubbles
// ---------------------------------------------------------------------------

void FUltraWireBubbleSystem::DrawBubbles(
	FSlateWindowElementList& DrawElements,
	int32 LayerID,
	float ZoomFactor,
	const TArray<FVector2D>& Positions,
	float Size,
	const FLinearColor& Color)
{
	if (Positions.Num() == 0 || Size < KINDA_SMALL_NUMBER)
	{
		return;
	}

	const FSlateBrush* CircleBrush = FCoreStyle::Get().GetBrush("WhiteBrush");
	if (!CircleBrush)
	{
		return;
	}

	const float ScaledSize = Size * ZoomFactor;
	const float HalfSize   = ScaledSize * 0.5f;

	for (const FVector2D& Pos : Positions)
	{
		// FPaintGeometry(DrawPosition, DrawSize, DrawScale)
		// DrawPosition is the top-left of the element in absolute space.
		FSlateDrawElement::MakeBox(
			DrawElements,
			static_cast<uint32>(LayerID),
			FPaintGeometry(
				FVector2D(Pos.X - HalfSize, Pos.Y - HalfSize),
				FVector2D(ScaledSize, ScaledSize),
				ZoomFactor),
			CircleBrush,
			ESlateDrawEffect::None,
			Color);
	}
}
