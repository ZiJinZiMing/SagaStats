// Copyright 2026 StraySpark. All Rights Reserved.

#include "UltraWireDrawingPolicy.h"
#include "UltraWireGeometry.h"
#include "UltraWireBubbleSystem.h"
#include "UltraWireCrossingDetector.h"
#include "UltraWireGlowRenderer.h"
#include "UltraWireLabelRenderer.h"
#include "UltraWireSettings.h"

#include "EdGraph/EdGraph.h"
#include "EdGraph/EdGraphPin.h"
#include "EdGraph/EdGraphSchema.h"
#include "Framework/Application/SlateApplication.h"
#include "Layout/PaintGeometry.h"
#include "Styling/CoreStyle.h"
#include "Fonts/SlateFontInfo.h"

// Base class protected member names (verified from ConnectionDrawingPolicy.h UE5.7):
//   WireLayerID      – back layer  (InBackLayerID  in ctor)
//   ArrowLayerID     – front layer (InFrontLayerID in ctor)
//   DrawElementsList – FSlateWindowElementList&
//   ZoomFactor       – float

// ---------------------------------------------------------------------------
// Construction
// ---------------------------------------------------------------------------

FUltraWireDrawingPolicy::FUltraWireDrawingPolicy(
	int32 InBackLayerID,
	int32 InFrontLayerID,
	float InZoomFactor,
	const FSlateRect& InClippingRect,
	FSlateWindowElementList& InDrawElements,
	UEdGraph* InGraphObj,
	EUltraWireGraphType InGraphType)
	: FConnectionDrawingPolicy(InBackLayerID, InFrontLayerID, InZoomFactor, InClippingRect, InDrawElements)
	, GraphType(InGraphType)
	, GraphObj(InGraphObj)
	, CurrentTime(FSlateApplication::Get().GetCurrentTime())
{
	// WirePaths / WireColors are populated incrementally as DrawConnection is called.
	// CachedAllottedGeometry is not used — we draw with FPaintGeometry() (absolute space).
}

// ---------------------------------------------------------------------------
// GetActiveProfile
// ---------------------------------------------------------------------------

const FUltraWireProfile& FUltraWireDrawingPolicy::GetActiveProfile() const
{
	const UUltraWireSettings* UWSettings = UUltraWireSettings::Get();
	if (!UWSettings)
	{
		static const FUltraWireProfile DefaultFallback;
		return DefaultFallback;
	}
	return UWSettings->GetProfileForGraphType(GraphType);
}

// ---------------------------------------------------------------------------
// DetermineWiringStyle
// ---------------------------------------------------------------------------

void FUltraWireDrawingPolicy::DetermineWiringStyle(
	UEdGraphPin* OutputPin,
	UEdGraphPin* InputPin,
	FConnectionParams& Params)
{
	// Call super first – resolves the base colour from the schema / pin types.
	FConnectionDrawingPolicy::DetermineWiringStyle(OutputPin, InputPin, Params);

	const UUltraWireSettings* UWSettings = UUltraWireSettings::Get();
	if (!UWSettings || !UWSettings->bEnabled)
	{
		return;
	}

	const FUltraWireProfile& Profile = GetActiveProfile();
	Params.WireThickness = Profile.WireThickness;
}

// ---------------------------------------------------------------------------
// ComputeWirePath
// ---------------------------------------------------------------------------

TArray<FVector2D> FUltraWireDrawingPolicy::ComputeWirePath(
	const FVector2D& Start, const FVector2D& End) const
{
	const FUltraWireProfile& Profile = GetActiveProfile();

	TArray<FVector2D> RawPath;

	switch (Profile.WireStyle)
	{
	case EUltraWireStyle::Manhattan:
		RawPath = FUltraWireGeometry::ComputeManhattanPath(Start, End);
		break;

	case EUltraWireStyle::Subway:
		RawPath = FUltraWireGeometry::ComputeSubwayPath(Start, End);
		break;

	case EUltraWireStyle::Freeform:
		RawPath = FUltraWireGeometry::ComputeFreeformPath(Start, End, Profile.FreeformAngle);
		break;

	case EUltraWireStyle::Default:
	default:
		// Caller falls back to super for Default style.
		RawPath.Add(Start);
		RawPath.Add(End);
		break;
	}

	// Apply corner rounding / chamfering.
	if (Profile.CornerStyle != EUltraWireCornerStyle::Sharp && Profile.CornerRadius > KINDA_SMALL_NUMBER)
	{
		RawPath = FUltraWireGeometry::ApplyCornerRounding(
			RawPath, Profile.CornerRadius, Profile.CornerStyle);
	}

	return RawPath;
}

// ---------------------------------------------------------------------------
// DrawWireLines
// ---------------------------------------------------------------------------

void FUltraWireDrawingPolicy::DrawWireLines(
	int32 LayerID,
	const TArray<FVector2D>& WirePath,
	const FLinearColor& Color,
	float Thickness)
{
	if (WirePath.Num() < 2)
	{
		return;
	}

	const FUltraWireProfile& Profile = GetActiveProfile();

	// When CrossingStyle is Gap we split the wire at crossing points so there
	// is a visible break at each intersection with a previously drawn wire.
	if (Profile.CrossingStyle == EUltraWireCrossingStyle::Gap && WirePaths.Num() > 0)
	{
		const float PathLength = FUltraWireGeometry::ComputePathLength(WirePath);
		if (PathLength < KINDA_SMALL_NUMBER)
		{
			return;
		}

		// Collect normalised T-values of all intersections with existing wires.
		TArray<float> GapCentres;

		for (const TArray<FVector2D>& OtherPath : WirePaths)
		{
			TArray<TArray<FVector2D>> Pair = { WirePath, OtherPath };
			TArray<FUltraWireCrossing> Crossings = FUltraWireCrossingDetector::DetectCrossings(Pair);

			for (const FUltraWireCrossing& C : Crossings)
			{
				if (C.WireIndexA != 0) { continue; }

				// Find the T of the crossing on WirePath.
				float Accumulated = 0.0f;
				for (int32 Seg = 0; Seg < WirePath.Num() - 1; ++Seg)
				{
					const float SegLen = FVector2D::Distance(WirePath[Seg], WirePath[Seg + 1]);
					if (SegLen < KINDA_SMALL_NUMBER) { continue; }

					const FVector2D SegDir = (WirePath[Seg + 1] - WirePath[Seg]) / SegLen;
					const float Dot = FVector2D::DotProduct(C.Position - WirePath[Seg], SegDir);
					if (Dot >= -KINDA_SMALL_NUMBER && Dot <= SegLen + KINDA_SMALL_NUMBER)
					{
						GapCentres.Add(
							FMath::Clamp((Accumulated + FMath::Clamp(Dot, 0.0f, SegLen)) / PathLength,
								0.0f, 1.0f));
						break;
					}
					Accumulated += SegLen;
				}
			}
		}

		if (GapCentres.Num() == 0)
		{
			// No gaps needed.
			FSlateDrawElement::MakeLines(
				DrawElementsList,
				static_cast<uint32>(LayerID),
				FPaintGeometry(),
				WirePath,
				ESlateDrawEffect::None,
				Color,
				true,
				Thickness);
			return;
		}

		GapCentres.Sort();

		const float GapHalfT = (Profile.CrossingSize * 0.5f) / FMath::Max(PathLength, 1.0f);

		// Build visible ranges between gaps.
		TArray<TTuple<float, float>> DrawRanges;
		float RangeStart = 0.0f;

		for (float GapT : GapCentres)
		{
			const float GapBegin = FMath::Max(0.0f, GapT - GapHalfT);
			const float GapEnd   = FMath::Min(1.0f, GapT + GapHalfT);

			if (GapBegin > RangeStart + KINDA_SMALL_NUMBER)
			{
				DrawRanges.Emplace(RangeStart, GapBegin);
			}
			RangeStart = GapEnd;
		}

		if (RangeStart < 1.0f - KINDA_SMALL_NUMBER)
		{
			DrawRanges.Emplace(RangeStart, 1.0f);
		}

		for (const auto& [T0, T1] : DrawRanges)
		{
			if (T1 - T0 < KINDA_SMALL_NUMBER) { continue; }

			const int32 NumSamples = FMath::Max(2,
				FMath::RoundToInt((T1 - T0) * static_cast<float>(WirePath.Num()) * 4));

			TArray<FVector2D> SubPath;
			SubPath.Reserve(NumSamples);

			for (int32 s = 0; s < NumSamples; ++s)
			{
				const float T = FMath::Lerp(T0, T1,
					static_cast<float>(s) / static_cast<float>(NumSamples - 1));
				SubPath.Add(FUltraWireGeometry::EvaluatePathAtT(WirePath, T));
			}

			FSlateDrawElement::MakeLines(
				DrawElementsList,
				static_cast<uint32>(LayerID),
				FPaintGeometry(),
				SubPath,
				ESlateDrawEffect::None,
				Color,
				true,
				Thickness);
		}
	}
	else
	{
		// Simple case: single polyline.
		FSlateDrawElement::MakeLines(
			DrawElementsList,
			static_cast<uint32>(LayerID),
			FPaintGeometry(),
			WirePath,
			ESlateDrawEffect::None,
			Color,
			true,
			Thickness);
	}
}

// ---------------------------------------------------------------------------
// DrawConnection  (FVector2f variant – primary in UE5.6+)
// ---------------------------------------------------------------------------

void FUltraWireDrawingPolicy::DrawConnection(
	int32 LayerID,
	const FVector2f& Start,
	const FVector2f& End,
	const FConnectionParams& Params)
{
	const UUltraWireSettings* UWSettings = UUltraWireSettings::Get();
	if (!UWSettings || !UWSettings->bEnabled)
	{
		FConnectionDrawingPolicy::DrawConnection(LayerID, Start, End, Params);
		return;
	}

	const FUltraWireProfile& Profile = GetActiveProfile();

	if (Profile.WireStyle == EUltraWireStyle::Default)
	{
		FConnectionDrawingPolicy::DrawConnection(LayerID, Start, End, Params);
		return;
	}

	// Upcast to double-precision for our geometry engine.
	const FVector2D StartD(Start.X, Start.Y);
	const FVector2D EndD(End.X, End.Y);

	// 1. Route the wire.
	TArray<FVector2D> WirePath = ComputeWirePath(StartD, EndD);

	if (WirePath.Num() < 2)
	{
		FConnectionDrawingPolicy::DrawConnection(LayerID, Start, End, Params);
		return;
	}

	// 2. Store for post-frame crossing detection.
	WirePaths.Add(WirePath);
	WireColors.Add(Params.WireColor);

	// 3. Glow pass (below the wire).
	if (Profile.bEnableGlow)
	{
		FUltraWireGlowRenderer::DrawGlowPass(
			DrawElementsList,
			WireLayerID,
			WirePath,
			Params.WireColor,
			Profile.GlowIntensity,
			Profile.GlowWidth,
			Profile.bGlowPulse,
			Profile.GlowPulseSpeed,
			CurrentTime);
	}

	// 4. Wire line segments.
	DrawWireLines(LayerID, WirePath, Params.WireColor, Params.WireThickness);

	// 5. Bubble animation.
	if (Profile.bEnableBubbles)
	{
		const TArray<FVector2D> BubblePositions = FUltraWireBubbleSystem::ComputeBubblePositions(
			WirePath,
			CurrentTime,
			Profile.BubbleSpeed,
			Profile.BubbleSpacing);

		FUltraWireBubbleSystem::DrawBubbles(
			DrawElementsList,
			ArrowLayerID,
			ZoomFactor,
			BubblePositions,
			Profile.BubbleSize,
			Params.WireColor);
	}

	// 6. Wire label (pin category name).
	if (Profile.bEnableAutoLabels
		&& Params.AssociatedPin1
		&& !Params.AssociatedPin1->PinType.PinCategory.IsNone())
	{
		const FText LabelText = FText::FromName(Params.AssociatedPin1->PinType.PinCategory);
		const FSlateFontInfo Font = FCoreStyle::GetDefaultFontStyle("Regular", 7);

		FUltraWireLabelRenderer::DrawWireLabel(
			DrawElementsList,
			ArrowLayerID,
			ZoomFactor,
			WirePath,
			LabelText,
			Font,
			FLinearColor(1.0f, 1.0f, 1.0f, 0.75f),
			/*bDrawBackground=*/true,
			FLinearColor(0.0f, 0.0f, 0.0f, 0.5f));
	}
}

// ---------------------------------------------------------------------------
// DrawCrossings
// ---------------------------------------------------------------------------

void FUltraWireDrawingPolicy::DrawCrossings()
{
	const FUltraWireProfile& Profile = GetActiveProfile();

	if (Profile.CrossingStyle == EUltraWireCrossingStyle::None || WirePaths.Num() < 2)
	{
		return;
	}

	const TArray<FUltraWireCrossing> Crossings =
		FUltraWireCrossingDetector::DetectCrossings(WirePaths);

	for (const FUltraWireCrossing& Crossing : Crossings)
	{
		const FLinearColor SymbolColor =
			WireColors.IsValidIndex(Crossing.WireIndexB)
			? WireColors[Crossing.WireIndexB]
			: FLinearColor::White;

		FUltraWireCrossingDetector::DrawCrossingSymbol(
			DrawElementsList,
			ArrowLayerID,
			ZoomFactor,
			Crossing.Position,
			Profile.CrossingStyle,
			Profile.CrossingSize,
			SymbolColor);
	}
}
