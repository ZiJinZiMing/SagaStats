// Copyright 2026 StraySpark. All Rights Reserved.

#include "UltraWireLabelRenderer.h"
#include "UltraWireGeometry.h"
#include "Rendering/DrawElements.h"
#include "Layout/PaintGeometry.h"
#include "Styling/CoreStyle.h"
#include "Framework/Application/SlateApplication.h"
#include "Fonts/FontMeasure.h"

// ---------------------------------------------------------------------------
// DrawWireLabel
// ---------------------------------------------------------------------------

void FUltraWireLabelRenderer::DrawWireLabel(
	FSlateWindowElementList& DrawElements,
	int32 LayerID,
	float ZoomFactor,
	const TArray<FVector2D>& WirePath,
	const FText& Text,
	const FSlateFontInfo& Font,
	const FLinearColor& TextColor,
	bool bDrawBackground,
	const FLinearColor& BackgroundColor,
	float VerticalOffset)
{
	if (WirePath.Num() < 2 || Text.IsEmpty())
	{
		return;
	}

	// Evaluate the midpoint of the wire path (T = 0.5).
	const FVector2D MidPoint = FUltraWireGeometry::EvaluatePathAtT(WirePath, 0.5f);

	// Measure the text so we can centre it and size the backing box.
	const TSharedRef<FSlateFontMeasure> FontMeasure =
		FSlateApplication::Get().GetRenderer()->GetFontMeasureService();

	const FVector2D TextSize = FontMeasure->Measure(Text, Font);
	if (TextSize.X < KINDA_SMALL_NUMBER || TextSize.Y < KINDA_SMALL_NUMBER)
	{
		return;
	}

	// Position the label so its visual centre aligns with MidPoint, with an
	// upward shift of VerticalOffset pixels.
	const FVector2D LabelOrigin(
		MidPoint.X - TextSize.X * 0.5f,
		MidPoint.Y - TextSize.Y * 0.5f + VerticalOffset);

	// --- Optional backing box ---
	if (bDrawBackground)
	{
		const FSlateBrush* BoxBrush = FCoreStyle::Get().GetBrush("WhiteBrush");
		if (BoxBrush)
		{
			const FVector2D BoxOrigin(
				LabelOrigin.X - BackgroundPaddingX,
				LabelOrigin.Y - BackgroundPaddingY);

			const FVector2D BoxSize(
				TextSize.X + BackgroundPaddingX * 2.0f,
				TextSize.Y + BackgroundPaddingY * 2.0f);

			FSlateDrawElement::MakeBox(
				DrawElements,
				static_cast<uint32>(LayerID),
				FPaintGeometry(BoxOrigin, BoxSize, ZoomFactor),
				BoxBrush,
				ESlateDrawEffect::None,
				BackgroundColor);
		}
	}

	// --- Text ---
	// MakeText with FText is defined inline in DrawElementTypes.h and delegates
	// to the FString overload after calling ToString().
	FSlateDrawElement::MakeText(
		DrawElements,
		static_cast<uint32>(LayerID + 1),   // text on top of backing box
		FPaintGeometry(LabelOrigin, TextSize, ZoomFactor),
		Text,
		Font,
		ESlateDrawEffect::None,
		TextColor);
}
