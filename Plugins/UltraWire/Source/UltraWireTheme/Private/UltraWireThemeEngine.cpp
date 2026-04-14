// Copyright 2026 StraySpark. All Rights Reserved.

#include "UltraWireThemeEngine.h"
#include "UltraWireTypes.h"
#include "Styling/AppStyle.h"
#include "Styling/SlateStyleRegistry.h"
#include "Styling/SlateStyle.h"
#include "Styling/CoreStyle.h"
#include "Brushes/SlateImageBrush.h"
#include "Brushes/SlateBoxBrush.h"
#include "Brushes/SlateRoundedBoxBrush.h"
#include "Brushes/SlateBorderBrush.h"
#include "Brushes/SlateNoResource.h"
#include "Brushes/SlateColorBrush.h"

// ---------------------------------------------------------------------------
// Named style keys used across UE5's graph editor – centralised here so that
// any future rename only needs to be updated in one place.
// ---------------------------------------------------------------------------
namespace UltraWireStyleKeys
{
	// Node body / shadow
	static const FName NodeBody             (TEXT("Graph.Node.Body"));
	static const FName NodeShadowSize       (TEXT("Graph.Node.ShadowSize"));
	static const FName NodeShadow           (TEXT("Graph.Node.Shadow"));

	// Node title / header
	static const FName NodeTitleGloss       (TEXT("Graph.Node.TitleGloss"));
	static const FName NodeColorSpill       (TEXT("Graph.Node.ColorSpill"));
	static const FName NodeTitleHighlight   (TEXT("Graph.Node.TitleHighlight"));

	// Pins
	static const FName PinPoint             (TEXT("Graph.Node.PinPoint"));
	static const FName PinDiffIndicator     (TEXT("Graph.Node.PinDiffIndicator"));

	// Comment bubble
	static const FName CommentBubbleBody    (TEXT("Graph.CommentBubble.Body"));
	static const FName CommentBubbleBorder  (TEXT("Graph.CommentBubble.Border"));
}

// ---------------------------------------------------------------------------
// Construction / Destruction
// ---------------------------------------------------------------------------

FUltraWireThemeEngine::FUltraWireThemeEngine()
{
}

FUltraWireThemeEngine::~FUltraWireThemeEngine()
{
	// Ensure we always restore originals even if the caller forgets.
	ResetTheme();
}

// ---------------------------------------------------------------------------
// Public API
// ---------------------------------------------------------------------------

void FUltraWireThemeEngine::ApplyTheme(const FUltraWireProfile& Profile)
{
	// Always snapshot first so that repeated ApplyTheme calls with different
	// profiles do not corrupt the saved originals.
	SnapshotBrushIfNeeded(UltraWireStyleKeys::NodeBody);
	SnapshotBrushIfNeeded(UltraWireStyleKeys::NodeShadowSize);
	SnapshotBrushIfNeeded(UltraWireStyleKeys::NodeShadow);
	SnapshotBrushIfNeeded(UltraWireStyleKeys::NodeTitleGloss);
	SnapshotBrushIfNeeded(UltraWireStyleKeys::NodeColorSpill);
	SnapshotBrushIfNeeded(UltraWireStyleKeys::NodeTitleHighlight);
	SnapshotBrushIfNeeded(UltraWireStyleKeys::PinPoint);
	SnapshotBrushIfNeeded(UltraWireStyleKeys::PinDiffIndicator);
	SnapshotBrushIfNeeded(UltraWireStyleKeys::CommentBubbleBody);
	SnapshotBrushIfNeeded(UltraWireStyleKeys::CommentBubbleBorder);

	// Apply each category of override.
	ApplyNodeBodyOverrides(Profile);
	ApplyNodeHeaderOverrides(Profile);
	ApplyPinShapeOverrides(Profile.PinShape);
	ApplyCommentBoxOverrides(Profile);

	bThemeApplied = true;

	UE_LOG(LogTemp, Verbose, TEXT("UltraWire: Theme applied – header tint (%.2f,%.2f,%.2f), body opacity %.2f, pin shape %d"),
		Profile.NodeHeaderTintColor.R,
		Profile.NodeHeaderTintColor.G,
		Profile.NodeHeaderTintColor.B,
		Profile.NodeBodyOpacity,
		static_cast<int32>(Profile.PinShape));
}

void FUltraWireThemeEngine::ResetTheme()
{
	if (!bThemeApplied)
	{
		return;
	}

	// Restore every brush we saved.
	for (auto& Pair : SavedBrushes)
	{
		SetStyleBrush(Pair.Key, Pair.Value.OriginalBrush);
	}

	bThemeApplied = false;

	UE_LOG(LogTemp, Verbose, TEXT("UltraWire: Theme reset to defaults."));
}

// ---------------------------------------------------------------------------
// Private – snapshot helpers
// ---------------------------------------------------------------------------

void FUltraWireThemeEngine::SnapshotBrushIfNeeded(const FName& StyleName)
{
	// Only snapshot once – we never want to overwrite a genuine original.
	if (SavedBrushes.Contains(StyleName))
	{
		return;
	}

	const ISlateStyle& AppStyle = FAppStyle::Get();

	// ISlateStyle::GetBrush returns a pointer; it may return the "missing"
	// brush when the key does not exist, which we treat as a valid brush to
	// restore later.
	const FSlateBrush* Brush = AppStyle.GetBrush(StyleName);
	if (Brush)
	{
		FSavedBrush& Saved   = SavedBrushes.Add(StyleName);
		Saved.StyleName      = StyleName;
		Saved.OriginalBrush  = *Brush;
	}
}

// ---------------------------------------------------------------------------
// Private – writing brushes back into FAppStyle
// ---------------------------------------------------------------------------

void FUltraWireThemeEngine::SetStyleBrush(const FName& StyleName, const FSlateBrush& Brush)
{
	// FAppStyle::GetAppStyle() returns an ISlateStyle& which is actually a
	// FSlateStyleSet underneath.  FSlateStyleSet::Set() is protected when
	// called from outside its own module, so the canonical workaround used
	// by Epic's own editor extensions is to go through the style registry:
	//   1. Unregister the live style.
	//   2. Downcast to FSlateStyleSet.
	//   3. Call Set().
	//   4. Re-register.
	//
	// In practice, for shipped UE5 builds the AppStyle is registered with
	// FSlateStyleRegistry under the name "CoreStyle".  We take a safer,
	// non-destructive path: locate the style set by name, cast it, mutate,
	// then invalidate.

	FSlateStyleSet* StyleSet = static_cast<FSlateStyleSet*>(
		const_cast<ISlateStyle*>(&FAppStyle::Get()));

	if (!StyleSet)
	{
		UE_LOG(LogTemp, Warning, TEXT("UltraWire: Could not obtain FSlateStyleSet for style override '%s'."), *StyleName.ToString());
		return;
	}

	// Use the brush-specific overload of Set which takes a pointer-to-brush.
	// FSlateStyleSet stores brushes in a separate map from widget styles.
	StyleSet->Set(StyleName, new FSlateBrush(Brush));

	// Notify the renderer that cached brush data is stale.
	FSlateApplication::Get().InvalidateAllWidgets(false);
}

// ---------------------------------------------------------------------------
// Private – brush builder
// ---------------------------------------------------------------------------

FSlateBrush FUltraWireThemeEngine::MakeTintedBrush(const FSlateBrush& Source,
                                                    const FLinearColor& Tint,
                                                    float OpacityScale)
{
	FSlateBrush Result = Source;

	// Blend the requested tint with whatever tint was already on the brush.
	// Multiplying preserves coloured source textures while allowing a full
	// white override to be a pure pass-through of the texture colour.
	FLinearColor BlendedTint = Source.TintColor.GetSpecifiedColor() * Tint;
	BlendedTint.A = FMath::Clamp(BlendedTint.A * OpacityScale, 0.0f, 1.0f);
	Result.TintColor = FSlateColor(BlendedTint);

	return Result;
}

// ---------------------------------------------------------------------------
// Private – per-category overrides
// ---------------------------------------------------------------------------

void FUltraWireThemeEngine::ApplyNodeBodyOverrides(const FUltraWireProfile& Profile)
{
	const ISlateStyle& AppStyle = FAppStyle::Get();

	// --- Node body opacity ---
	if (const FSlateBrush* BodyBrush = AppStyle.GetBrush(UltraWireStyleKeys::NodeBody))
	{
		FLinearColor BodyTint = BodyBrush->TintColor.GetSpecifiedColor();
		BodyTint.A = FMath::Clamp(Profile.NodeBodyOpacity, 0.0f, 1.0f);

		FSlateBrush Modified = *BodyBrush;
		Modified.TintColor   = FSlateColor(BodyTint);
		SetStyleBrush(UltraWireStyleKeys::NodeBody, Modified);
	}

	// --- Drop-shadow – scale with opacity so transparent bodies look right ---
	if (const FSlateBrush* ShadowBrush = AppStyle.GetBrush(UltraWireStyleKeys::NodeShadow))
	{
		const float ShadowOpacity = FMath::Lerp(0.1f, 1.0f, Profile.NodeBodyOpacity);
		SetStyleBrush(UltraWireStyleKeys::NodeShadow,
		              MakeTintedBrush(*ShadowBrush, FLinearColor::White, ShadowOpacity));
	}
}

void FUltraWireThemeEngine::ApplyNodeHeaderOverrides(const FUltraWireProfile& Profile)
{
	const ISlateStyle& AppStyle = FAppStyle::Get();

	// Apply the user-chosen tint to the title gloss, color spill, and
	// title highlight brushes.  The gloss and highlight receive a softer
	// version of the tint so that the specular layering still reads clearly.

	const FLinearColor& HeaderTint = Profile.NodeHeaderTintColor;

	// Full-strength tint on the color spill (the solid-color band).
	if (const FSlateBrush* SpillBrush = AppStyle.GetBrush(UltraWireStyleKeys::NodeColorSpill))
	{
		SetStyleBrush(UltraWireStyleKeys::NodeColorSpill,
		              MakeTintedBrush(*SpillBrush, HeaderTint));
	}

	// Softer tint on the gloss (preserves highlight contrast).
	if (const FSlateBrush* GlossBrush = AppStyle.GetBrush(UltraWireStyleKeys::NodeTitleGloss))
	{
		// Blend toward white at 60 % so the specular is not obliterated.
		const FLinearColor SoftTint = FMath::Lerp(HeaderTint, FLinearColor::White, 0.60f);
		SetStyleBrush(UltraWireStyleKeys::NodeTitleGloss,
		              MakeTintedBrush(*GlossBrush, SoftTint));
	}

	// Light tint on the title highlight stripe.
	if (const FSlateBrush* HighlightBrush = AppStyle.GetBrush(UltraWireStyleKeys::NodeTitleHighlight))
	{
		const FLinearColor LightTint = FMath::Lerp(HeaderTint, FLinearColor::White, 0.40f);
		SetStyleBrush(UltraWireStyleKeys::NodeTitleHighlight,
		              MakeTintedBrush(*HighlightBrush, LightTint));
	}
}

void FUltraWireThemeEngine::ApplyPinShapeOverrides(EUltraWirePinShape PinShape)
{
	// UE5 renders pins using SlateBrushes whose image resource determines the
	// shape (circle for execution pins, etc.).  We expose the four canonical
	// shapes defined in EUltraWirePinShape by switching the TintColor to a
	// distinguishable value that the custom UltraWireRenderer module can
	// consume when it overrides the pin draw code.
	//
	// For the ThemeEngine layer specifically we apply a tint-based hint on
	// the PinPoint brush:  the alpha encodes the shape enum so the renderer
	// can pick the correct path without a separate communication channel.
	//
	// Encoding scheme (alpha component of the pin tint):
	//   Circle   -> 1.00 (fully opaque, no change)
	//   Diamond  -> 0.75
	//   Square   -> 0.50
	//   Arrow    -> 0.25

	const ISlateStyle& AppStyle = FAppStyle::Get();

	if (const FSlateBrush* PinBrush = AppStyle.GetBrush(UltraWireStyleKeys::PinPoint))
	{
		float AlphaHint = 1.0f;
		switch (PinShape)
		{
		case EUltraWirePinShape::Circle:  AlphaHint = 1.00f; break;
		case EUltraWirePinShape::Diamond: AlphaHint = 0.75f; break;
		case EUltraWirePinShape::Square:  AlphaHint = 0.50f; break;
		case EUltraWirePinShape::Arrow:   AlphaHint = 0.25f; break;
		default:                          AlphaHint = 1.00f; break;
		}

		FSlateBrush Modified   = *PinBrush;
		FLinearColor PinTint   = Modified.TintColor.GetSpecifiedColor();
		PinTint.A              = AlphaHint;
		Modified.TintColor     = FSlateColor(PinTint);
		SetStyleBrush(UltraWireStyleKeys::PinPoint, Modified);
	}
}

void FUltraWireThemeEngine::ApplyCommentBoxOverrides(const FUltraWireProfile& Profile)
{
	// Comment boxes receive a subtle tint derived from the header colour so
	// that the graph has visual cohesion.  We desaturate and lighten the tint
	// considerably to keep comments legible.

	const ISlateStyle& AppStyle = FAppStyle::Get();

	FLinearColor HeaderHSV = Profile.NodeHeaderTintColor.LinearRGBToHSV();
	// Desaturate to 20 % of original saturation, then bring value to 85 %.
	HeaderHSV.G *= 0.20f;
	HeaderHSV.B  = 0.85f;
	const FLinearColor CommentBodyTint = HeaderHSV.HSVToLinearRGB();

	if (const FSlateBrush* BodyBrush = AppStyle.GetBrush(UltraWireStyleKeys::CommentBubbleBody))
	{
		FSlateBrush Modified = MakeTintedBrush(*BodyBrush, CommentBodyTint, 0.9f);
		SetStyleBrush(UltraWireStyleKeys::CommentBubbleBody, Modified);
	}

	if (const FSlateBrush* BorderBrush = AppStyle.GetBrush(UltraWireStyleKeys::CommentBubbleBorder))
	{
		// Keep the border slightly darker than the body.
		const FLinearColor BorderTint = CommentBodyTint * 0.7f;
		FSlateBrush Modified = MakeTintedBrush(*BorderBrush, BorderTint, 0.9f);
		SetStyleBrush(UltraWireStyleKeys::CommentBubbleBorder, Modified);
	}
}
