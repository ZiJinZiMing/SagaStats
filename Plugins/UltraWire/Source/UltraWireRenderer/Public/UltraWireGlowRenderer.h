// Copyright 2026 StraySpark. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Rendering/DrawElements.h"

/**
 * FUltraWireGlowRenderer
 *
 * Renders a multi-layer additive-blended "glow" effect around a wire path.
 *
 * The glow is achieved by drawing the wire path multiple times with
 * progressively wider line thickness and decreasing alpha, mimicking the
 * look of an emissive bloom without a full post-process pass.
 *
 * Pulse animation is supported via a sinusoidal intensity modulator driven by
 * the caller-supplied Time value.
 *
 * Coordinate convention:
 *   WirePath is in graph panel absolute (screen) space, matching the
 *   coordinates passed to FConnectionDrawingPolicy::DrawConnection.
 *   Drawing uses FPaintGeometry() (identity/null geometry) since MakeLines
 *   treats the point array as already being in absolute space.
 */
class ULTRAWIRERENDERER_API FUltraWireGlowRenderer
{
public:
	/**
	 * DrawGlowPass
	 *
	 * Renders the glow layers for a wire path.  Should be drawn on a layer
	 * BELOW the main wire so the core wire appears on top.
	 *
	 * @param DrawElements   Slate draw list to append to.
	 * @param LayerID        Starting layer ID; inner glow layers use higher IDs.
	 * @param WirePath       Ordered waypoints of the wire in panel-absolute space.
	 * @param BaseColor      Core colour of the glow (full-intensity centre).
	 * @param Intensity      Multiplier on the glow alpha [0..5].
	 * @param Width          Total glow width (outermost radius) in panel units.
	 * @param bPulse         Whether to animate the intensity sinusoidally.
	 * @param PulseSpeed     Frequency of the pulse in Hz (cycles per second).
	 * @param Time           Current time in seconds (used for pulse animation).
	 */
	static void DrawGlowPass(
		FSlateWindowElementList& DrawElements,
		int32 LayerID,
		const TArray<FVector2D>& WirePath,
		const FLinearColor& BaseColor,
		float Intensity,
		float Width,
		bool bPulse,
		float PulseSpeed,
		double Time);

private:
	/** Number of concentric glow layers drawn around the wire core. */
	static constexpr int32 NumGlowLayers = 4;

	/**
	 * Returns the current intensity multiplier taking pulse animation into
	 * account.  Returns Intensity unchanged if bPulse is false.
	 */
	static float ComputeCurrentIntensity(float Intensity, bool bPulse, float PulseSpeed, double Time);
};
