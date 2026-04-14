// Copyright 2026 StraySpark. All Rights Reserved.

#include "UltraWireGlowRenderer.h"
#include "Layout/PaintGeometry.h"
#include "Math/UnrealMathUtility.h"

// ---------------------------------------------------------------------------
// ComputeCurrentIntensity
// ---------------------------------------------------------------------------

float FUltraWireGlowRenderer::ComputeCurrentIntensity(
	float Intensity, bool bPulse, float PulseSpeed, double Time)
{
	if (!bPulse || PulseSpeed < KINDA_SMALL_NUMBER)
	{
		return Intensity;
	}

	// Sinusoidal modulation: maps to [0.25, 1.0] so the glow never fully
	// disappears at the trough.
	const float Phase = static_cast<float>(Time) * PulseSpeed * 2.0f * PI;
	const float Wave  = FMath::Sin(Phase) * 0.5f + 0.5f;  // [0, 1]
	const float Mod   = 0.25f + Wave * 0.75f;              // [0.25, 1.0]

	return Intensity * Mod;
}

// ---------------------------------------------------------------------------
// DrawGlowPass
// ---------------------------------------------------------------------------

void FUltraWireGlowRenderer::DrawGlowPass(
	FSlateWindowElementList& DrawElements,
	int32 LayerID,
	const TArray<FVector2D>& WirePath,
	const FLinearColor& BaseColor,
	float Intensity,
	float Width,
	bool bPulse,
	float PulseSpeed,
	double Time)
{
	if (WirePath.Num() < 2 || Intensity < KINDA_SMALL_NUMBER || Width < KINDA_SMALL_NUMBER)
	{
		return;
	}

	const float CurrentIntensity = ComputeCurrentIntensity(Intensity, bPulse, PulseSpeed, Time);

	// Draw from the outermost (widest, most transparent) layer inward so that
	// inner layers are rendered on top, building up the bright centre.
	//
	// N=0 is the outermost halo; N=(NumGlowLayers-1) is the inner bright fringe
	// just outside the solid wire line.
	for (int32 N = 0; N < NumGlowLayers; ++N)
	{
		const float Frac = static_cast<float>(N) / static_cast<float>(NumGlowLayers - 1);

		// Thickness: outermost = Width, innermost = Width / NumGlowLayers.
		const float LayerWidth = Width * FMath::Lerp(1.0f, 1.0f / NumGlowLayers, Frac);

		// Alpha grows toward the centre (Frac -> 1).  Squared falloff produces
		// a soft halo that brightens sharply at the core.
		const float Alpha = FMath::Pow(Frac, 2.0f) * CurrentIntensity * 0.6f;

		if (Alpha < KINDA_SMALL_NUMBER)
		{
			continue;
		}

		FLinearColor LayerColor = BaseColor;
		LayerColor.A = FMath::Clamp(Alpha, 0.0f, 1.0f);

		// MakeLines with a null/identity FPaintGeometry treats the point
		// array as being in absolute panel space, which is exactly what we want.
		FSlateDrawElement::MakeLines(
			DrawElements,
			static_cast<uint32>(LayerID + N),
			FPaintGeometry(),
			WirePath,
			ESlateDrawEffect::None,
			LayerColor,
			/*bAntialias=*/true,
			LayerWidth);
	}
}
