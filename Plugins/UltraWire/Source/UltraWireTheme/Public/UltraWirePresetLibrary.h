// Copyright 2026 StraySpark. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UltraWireTypes.h"

/**
 * FUltraWirePresetLibrary
 *
 * A stateless collection of factory functions that return fully-configured
 * FUltraWirePreset values.  None of the functions allocate heap memory or
 * hold any state; they are safe to call from any thread.
 *
 * Built-in presets
 * ----------------
 *  Default             – Subtle improvements over vanilla UE.  Serves as a
 *                        sensible baseline that designers can tweak.
 *  CircuitBoard        – Green-tinted PCB aesthetic inspired by hardware
 *                        schematics.
 *  NeonCyberpunk       – Dark background with bright neon glow accents.
 *  CleanProfessional   – Minimal, high-contrast, accessibility-friendly.
 *  RetroTerminal       – Amber/green monochrome reminiscent of CRT terminals.
 *
 * Adding new built-in presets
 * ---------------------------
 * 1. Declare a new static function here following the same signature pattern.
 * 2. Implement it in UltraWirePresetLibrary.cpp.
 * 3. Add the call to GetAllBuiltInPresets() so the UI picks it up
 *    automatically.
 */
class ULTRAWIRETHEME_API FUltraWirePresetLibrary
{
public:

	// ------------------------------------------------------------------
	// Individual preset factories
	// ------------------------------------------------------------------

	/**
	 * Subtle improvements over vanilla UE5: slightly rounded wire corners,
	 * clean pin dots, gentle body transparency.  All glow and bubble effects
	 * are disabled so it blends into any existing project without disruption.
	 */
	static FUltraWirePreset GetDefaultPreset();

	/**
	 * Green-tinted PCB aesthetic.  Nodes have a deep green header, wires use
	 * a bright PCB-trace green, and the Manhattan routing style enforces the
	 * right-angle traces found on real circuit boards.
	 */
	static FUltraWirePreset GetCircuitBoardPreset();

	/**
	 * Dark neon cyberpunk aesthetic.  Near-black node bodies with bright cyan
	 * and magenta accents, active glow on wires, and animated flow bubbles.
	 */
	static FUltraWirePreset GetNeonCyberpunkPreset();

	/**
	 * Minimal, high-contrast professional look.  Pure white node headers,
	 * maximum body opacity, no glow or animations.  Designed for readability
	 * in screen-share and presentation contexts.
	 */
	static FUltraWirePreset GetCleanProfessionalPreset();

	/**
	 * Retro terminal aesthetic using an amber/green monochrome palette.
	 * Nodes have an amber header, wires are rendered in phosphor green, and
	 * a subtle scanline-like effect is approximated via body opacity pulsing.
	 */
	static FUltraWirePreset GetRetroTerminalPreset();

	// ------------------------------------------------------------------
	// Aggregate accessor – used by the settings UI preset browser
	// ------------------------------------------------------------------

	/**
	 * Returns all built-in presets in the canonical display order.
	 * The order matches the visual order in the preset picker dropdown.
	 */
	static TArray<FUltraWirePreset> GetAllBuiltInPresets();

private:

	// No instantiation – pure static library.
	FUltraWirePresetLibrary() = delete;
};
