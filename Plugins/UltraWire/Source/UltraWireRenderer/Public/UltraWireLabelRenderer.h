// Copyright 2026 StraySpark. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Rendering/DrawElements.h"
#include "Fonts/SlateFontInfo.h"

/**
 * FUltraWireLabelRenderer
 *
 * Renders a text label at the midpoint of a wire path.
 *
 * The label is drawn horizontally regardless of the wire direction at its
 * midpoint, and is offset slightly above the wire so it does not overlap the
 * line.  A small translucent backing box can optionally be drawn beneath the
 * text to improve readability over busy graph backgrounds.
 *
 * Coordinate convention:
 *   WirePath is in graph panel absolute (screen) space, matching the
 *   coordinates passed to FConnectionDrawingPolicy::DrawConnection.
 *   Drawing uses FPaintGeometry(Position, Size, Scale) directly.
 */
class ULTRAWIRERENDERER_API FUltraWireLabelRenderer
{
public:
	/**
	 * DrawWireLabel
	 *
	 * Renders Text centred on the midpoint of WirePath.
	 *
	 * @param DrawElements     Slate draw list to append to.
	 * @param LayerID          Slate layer to draw on.
	 * @param ZoomFactor       Current zoom factor for scaling.
	 * @param WirePath         Ordered waypoints of the wire in panel-absolute space.
	 * @param Text             The label string to render.
	 * @param Font             Slate font to use for rendering.
	 * @param TextColor        Colour of the label text.
	 * @param bDrawBackground  Whether to draw a translucent backing box.
	 * @param BackgroundColor  Colour of the optional backing box.
	 * @param VerticalOffset   Pixels to shift the label above the wire centre.
	 */
	static void DrawWireLabel(
		FSlateWindowElementList& DrawElements,
		int32 LayerID,
		float ZoomFactor,
		const TArray<FVector2D>& WirePath,
		const FText& Text,
		const FSlateFontInfo& Font,
		const FLinearColor& TextColor = FLinearColor::White,
		bool bDrawBackground = true,
		const FLinearColor& BackgroundColor = FLinearColor(0.0f, 0.0f, 0.0f, 0.55f),
		float VerticalOffset = -10.0f);

private:
	/** Horizontal padding applied to each side of the backing box. */
	static constexpr float BackgroundPaddingX = 4.0f;

	/** Vertical padding applied to the top and bottom of the backing box. */
	static constexpr float BackgroundPaddingY = 2.0f;
};
