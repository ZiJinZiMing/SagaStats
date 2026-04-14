// Copyright 2026 StraySpark. All Rights Reserved.

#include "UltraWirePresetLibrary.h"
#include "UltraWireTypes.h"

// ---------------------------------------------------------------------------
// Internal helper – builds the FUltraWirePreset shell so the factory bodies
// only need to fill in the profile and the descriptive strings.
// ---------------------------------------------------------------------------
namespace
{
	FUltraWirePreset MakePreset(const FString& Name,
	                            const FString& Description,
	                            const FString& Author,
	                            const FUltraWireProfile& Profile)
	{
		FUltraWirePreset Preset;
		Preset.PresetName  = Name;
		Preset.Description = Description;
		Preset.Author      = Author;
		Preset.Profile     = Profile;
		return Preset;
	}
}

// ---------------------------------------------------------------------------
// Default
// ---------------------------------------------------------------------------

FUltraWirePreset FUltraWirePresetLibrary::GetDefaultPreset()
{
	FUltraWireProfile P;

	// Wire routing – Manhattan with soft rounded corners.
	P.WireStyle           = EUltraWireStyle::Manhattan;
	P.CornerStyle         = EUltraWireCornerStyle::Rounded;
	P.CornerRadius        = 8.0f;
	P.WireThickness       = 1.5f;

	// Ribbons / crossings – off by default.
	P.bEnableRibbons      = false;
	P.CrossingStyle       = EUltraWireCrossingStyle::None;

	// Bubbles / glow – off.
	P.bEnableBubbles      = false;
	P.bEnableGlow         = false;

	// Smart routing – off; users enable per-project.
	P.bEnableSmartRouting = false;

	// Heatmap – off.
	P.bEnableHeatmap      = false;

	// Node theming – on, but with conservative settings.
	P.bEnableNodeTheming  = true;
	P.NodeBodyOpacity     = 0.92f;
	P.NodeHeaderTintColor = FLinearColor(1.0f, 1.0f, 1.0f, 1.0f);  // White (neutral)
	P.NodeCornerRadius    = 4.0f;
	P.PinShape            = EUltraWirePinShape::Circle;

	// Labels – off.
	P.bEnableAutoLabels   = false;

	// Minimap – off; large projects may want it enabled.
	P.bEnableMinimap      = false;
	P.MinimapPosition     = EUltraWireMinimapPosition::BottomRight;
	P.MinimapSize         = 200.0f;
	P.MinimapOpacity      = 0.8f;

	return MakePreset(
		TEXT("Default"),
		TEXT("Subtle improvements over vanilla UE5. Rounded wire corners, clean "
		     "pin dots, and a touch of body transparency. All animations and glow "
		     "effects are disabled so existing projects are not visually disrupted."),
		TEXT("UltraWire"),
		P);
}

// ---------------------------------------------------------------------------
// Circuit Board
// ---------------------------------------------------------------------------

FUltraWirePreset FUltraWirePresetLibrary::GetCircuitBoardPreset()
{
	FUltraWireProfile P;

	// Manhattan routing is mandatory for the PCB-trace aesthetic.
	P.WireStyle           = EUltraWireStyle::Manhattan;
	P.CornerStyle         = EUltraWireCornerStyle::Sharp;   // PCB traces have sharp corners.
	P.CornerRadius        = 0.0f;
	P.WireThickness       = 1.25f;

	// Ribbons make multi-wire bundles look like trace groups.
	P.bEnableRibbons      = true;
	P.RibbonOffset        = 3.0f;

	// Gap crossings mimic the overlapping-trace convention on schematics.
	P.CrossingStyle       = EUltraWireCrossingStyle::Gap;
	P.CrossingSize        = 6.0f;

	// No bubbles or glow – real PCBs have none.
	P.bEnableBubbles      = false;
	P.bEnableGlow         = false;

	// Smart routing keeps traces from crossing needlessly.
	P.bEnableSmartRouting = true;
	P.RoutingGridSize     = 15;
	P.RoutingNodePadding  = 8.0f;

	// No heatmap.
	P.bEnableHeatmap      = false;

	// Node theming – deep PCB green header.
	//   Solder-mask green:  #1A472A  ->  linear (0.006, 0.066, 0.025)
	P.bEnableNodeTheming  = true;
	P.NodeBodyOpacity     = 0.96f;
	P.NodeHeaderTintColor = FLinearColor(0.006f, 0.066f, 0.025f, 1.0f);
	P.NodeCornerRadius    = 2.0f;   // PCB pads have small radii.
	P.PinShape            = EUltraWirePinShape::Square;  // Square pads.

	P.bEnableAutoLabels   = true;   // Net labels are part of schematics.

	// Minimap – useful on large PCB-style graphs.
	P.bEnableMinimap      = true;
	P.MinimapPosition     = EUltraWireMinimapPosition::BottomRight;
	P.MinimapSize         = 220.0f;
	P.MinimapOpacity      = 0.85f;

	return MakePreset(
		TEXT("Circuit Board"),
		TEXT("Green-tinted PCB aesthetic inspired by hardware schematics. "
		     "Manhattan routing, sharp corners, and square pin pads produce "
		     "right-angle traces reminiscent of a printed circuit board. "
		     "Net labels and gap crossings complete the look."),
		TEXT("UltraWire"),
		P);
}

// ---------------------------------------------------------------------------
// Neon Cyberpunk
// ---------------------------------------------------------------------------

FUltraWirePreset FUltraWirePresetLibrary::GetNeonCyberpunkPreset()
{
	FUltraWireProfile P;

	// Bezier wires with soft corners feel organic and futuristic.
	P.WireStyle           = EUltraWireStyle::Default;
	P.CornerStyle         = EUltraWireCornerStyle::Rounded;
	P.CornerRadius        = 14.0f;
	P.WireThickness       = 2.0f;

	P.bEnableRibbons      = false;
	P.CrossingStyle       = EUltraWireCrossingStyle::Arc;
	P.CrossingSize        = 10.0f;

	// Animated bubbles reinforce the data-flow metaphor.
	P.bEnableBubbles      = true;
	P.BubbleSpeed         = 160.0f;
	P.BubbleSize          = 3.5f;
	P.BubbleSpacing       = 60.0f;

	// Strong glow with pulse.
	P.bEnableGlow         = true;
	P.GlowIntensity       = 2.5f;
	P.GlowWidth           = 10.0f;
	P.bGlowPulse          = true;
	P.GlowPulseSpeed      = 1.5f;

	P.bEnableSmartRouting = false;

	// Heatmap complements the neon style well.
	P.bEnableHeatmap      = true;
	P.HeatmapColdColor    = FLinearColor(0.0f, 0.8f, 1.0f, 1.0f);   // Cyan
	P.HeatmapHotColor     = FLinearColor(1.0f, 0.0f, 0.6f, 1.0f);   // Magenta

	// Node theming – near-black body, vivid cyan header.
	//   Neon cyan: #00FFFF -> linear (0.0, 1.0, 1.0)
	P.bEnableNodeTheming  = true;
	P.NodeBodyOpacity     = 0.78f;   // Semi-transparent for depth layering.
	P.NodeHeaderTintColor = FLinearColor(0.0f, 0.85f, 1.0f, 1.0f);
	P.NodeCornerRadius    = 6.0f;
	P.PinShape            = EUltraWirePinShape::Diamond;  // Edgy diamond pins.

	P.bEnableAutoLabels   = false;

	P.bEnableMinimap      = true;
	P.MinimapPosition     = EUltraWireMinimapPosition::BottomRight;
	P.MinimapSize         = 240.0f;
	P.MinimapOpacity      = 0.75f;

	return MakePreset(
		TEXT("Neon Cyberpunk"),
		TEXT("Dark atmosphere with bright cyan and magenta neon glow accents. "
		     "Animated flow bubbles, pulsing wire glow, and heatmap coloring "
		     "combine to produce a futuristic data-visualization aesthetic."),
		TEXT("UltraWire"),
		P);
}

// ---------------------------------------------------------------------------
// Clean Professional
// ---------------------------------------------------------------------------

FUltraWirePreset FUltraWirePresetLibrary::GetCleanProfessionalPreset()
{
	FUltraWireProfile P;

	// Straight Manhattan wires with chamfered corners – structured but not sterile.
	P.WireStyle           = EUltraWireStyle::Manhattan;
	P.CornerStyle         = EUltraWireCornerStyle::Chamfered;
	P.CornerRadius        = 6.0f;
	P.WireThickness       = 1.25f;

	// No decorative elements – clean above all else.
	P.bEnableRibbons      = false;
	P.CrossingStyle       = EUltraWireCrossingStyle::None;
	P.bEnableBubbles      = false;
	P.bEnableGlow         = false;
	P.bEnableSmartRouting = true;
	P.RoutingGridSize     = 20;
	P.RoutingNodePadding  = 12.0f;
	P.bEnableHeatmap      = false;

	// Node theming – pure white tint (maximum contrast), full opacity.
	P.bEnableNodeTheming  = true;
	P.NodeBodyOpacity     = 1.0f;
	P.NodeHeaderTintColor = FLinearColor(1.0f, 1.0f, 1.0f, 1.0f);
	P.NodeCornerRadius    = 3.0f;
	P.PinShape            = EUltraWirePinShape::Circle;

	P.bEnableAutoLabels   = true;  // Labels aid readability in presentations.

	P.bEnableMinimap      = true;
	P.MinimapPosition     = EUltraWireMinimapPosition::TopRight;
	P.MinimapSize         = 180.0f;
	P.MinimapOpacity      = 0.9f;

	return MakePreset(
		TEXT("Clean Professional"),
		TEXT("Minimal, high-contrast design optimised for readability in screen "
		     "shares, presentations, and code reviews. No glow, no animations – "
		     "just clear structure. Chamfered corners and auto-labels keep the "
		     "graph organised without visual noise."),
		TEXT("UltraWire"),
		P);
}

// ---------------------------------------------------------------------------
// Retro Terminal
// ---------------------------------------------------------------------------

FUltraWirePreset FUltraWirePresetLibrary::GetRetroTerminalPreset()
{
	FUltraWireProfile P;

	// Subway (45-degree) routing – old terminal UIs drew diagonal connectors.
	P.WireStyle           = EUltraWireStyle::Subway;
	P.CornerStyle         = EUltraWireCornerStyle::Sharp;
	P.CornerRadius        = 0.0f;
	P.WireThickness       = 1.0f;

	P.bEnableRibbons      = false;
	P.CrossingStyle       = EUltraWireCrossingStyle::Circle;  // Solder-dot crossings.
	P.CrossingSize        = 5.0f;

	// Slow bubbles simulate cursor blink / scan-line data flow.
	P.bEnableBubbles      = true;
	P.BubbleSpeed         = 60.0f;
	P.BubbleSize          = 2.0f;
	P.BubbleSpacing       = 80.0f;

	// Soft phosphor glow – low intensity to stay authentic.
	P.bEnableGlow         = true;
	P.GlowIntensity       = 0.8f;
	P.GlowWidth           = 4.0f;
	P.bGlowPulse          = false;

	P.bEnableSmartRouting = false;
	P.bEnableHeatmap      = false;

	// Node theming – amber header (P3 phosphor), dark body.
	//   Phosphor amber: #FFB300 -> linear (1.0, 0.49, 0.0) approximately
	//   (using sRGB-to-linear approximation)
	P.bEnableNodeTheming  = true;
	P.NodeBodyOpacity     = 0.88f;
	P.NodeHeaderTintColor = FLinearColor(0.855f, 0.330f, 0.0f, 1.0f);   // Amber
	P.NodeCornerRadius    = 0.0f;   // CRT displays had square bezels.
	P.PinShape            = EUltraWirePinShape::Square;  // Blocky, pixelated feel.

	P.bEnableAutoLabels   = false;

	P.bEnableMinimap      = true;
	P.MinimapPosition     = EUltraWireMinimapPosition::BottomLeft;
	P.MinimapSize         = 160.0f;
	P.MinimapOpacity      = 0.7f;

	return MakePreset(
		TEXT("Retro Terminal"),
		TEXT("Amber and phosphor-green monochrome aesthetic inspired by 1980s "
		     "CRT terminals. Square corners, solder-dot crossings, slow data-flow "
		     "bubbles, and a warm amber glow recreate the look of early hardware "
		     "design tools."),
		TEXT("UltraWire"),
		P);
}

// ---------------------------------------------------------------------------
// Aggregate accessor
// ---------------------------------------------------------------------------

TArray<FUltraWirePreset> FUltraWirePresetLibrary::GetAllBuiltInPresets()
{
	TArray<FUltraWirePreset> Presets;
	Presets.Reserve(5);

	Presets.Add(GetDefaultPreset());
	Presets.Add(GetCircuitBoardPreset());
	Presets.Add(GetNeonCyberpunkPreset());
	Presets.Add(GetCleanProfessionalPreset());
	Presets.Add(GetRetroTerminalPreset());

	return Presets;
}
