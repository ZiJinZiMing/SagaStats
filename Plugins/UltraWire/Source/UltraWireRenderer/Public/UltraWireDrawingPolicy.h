// Copyright 2026 StraySpark. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "ConnectionDrawingPolicy.h"
#include "UltraWireTypes.h"

class UUltraWireSettings;
class FUltraWireBubbleSystem;
class FUltraWireCrossingDetector;
class FUltraWireGlowRenderer;
class FUltraWireLabelRenderer;

/**
 * FUltraWireDrawingPolicy
 *
 * Main Blueprint / editor graph wire drawing policy for the UltraWire plugin.
 *
 * Inherits FConnectionDrawingPolicy (the public base available in UE5.5+
 * without requiring internal Kismet headers).  Overrides DrawConnection() to
 * route wires through the UltraWire geometry engine and applies all visual
 * effects (glow, bubbles, crossing symbols, labels) configured via
 * UUltraWireSettings.
 *
 * Wire path storage:
 *   Every time DrawConnection() is called it appends the computed wire path to
 *   WirePaths so that the crossing detector can operate on the full frame's
 *   wire set after all wires have been drawn.  DrawCrossings() must be called
 *   explicitly after the normal draw loop (done by the owning SGraphPanel via
 *   the factory).
 *
 * UE5.6+ API note:
 *   FConnectionDrawingPolicy::DrawConnection() changed from FVector2D to FVector2f
 *   in UE5.6.  We override the FVector2f variant (the non-deprecated one) and
 *   also override the deprecated FVector2D variant so older engine versions
 *   remain compatible.  Internally all our geometry is computed in FVector2D
 *   (double precision) to match graph-panel coordinates.
 *
 * Base class protected member names (UE5.6+):
 *   WireLayerID   – corresponds to InBackLayerID  (lower layer)
 *   ArrowLayerID  – corresponds to InFrontLayerID (upper layer)
 *   DrawElementsList – the FSlateWindowElementList& passed to the ctor
 *
 * Thread safety:
 *   Instances are created per-frame per graph panel by the factory and are
 *   used only on the game thread.  No synchronisation is required.
 */
class ULTRAWIRERENDERER_API FUltraWireDrawingPolicy : public FConnectionDrawingPolicy
{
public:
	FUltraWireDrawingPolicy(
		int32 InBackLayerID,
		int32 InFrontLayerID,
		float InZoomFactor,
		const FSlateRect& InClippingRect,
		FSlateWindowElementList& InDrawElements,
		UEdGraph* InGraphObj,
		EUltraWireGraphType InGraphType);

	virtual ~FUltraWireDrawingPolicy() override = default;

	// -------------------------------------------------------------------------
	// FConnectionDrawingPolicy overrides
	// -------------------------------------------------------------------------

	/**
	 * DrawConnection (FVector2f variant – primary, UE5.6+)
	 *
	 * Routes the wire between Start and End using the active style from
	 * UUltraWireSettings, then renders:
	 *   1. Glow pass (if enabled) below the wire
	 *   2. Wire line segments (using MakeLines)
	 *   3. Bubble animation (if enabled)
	 *   4. Wire label (if auto-labels enabled)
	 *
	 * Also stores the computed path in WirePaths for later crossing detection.
	 */
	virtual void DrawConnection(
		int32 LayerID,
		const FVector2f& Start,
		const FVector2f& End,
		const FConnectionParams& Params) override;

	/**
	 * DetermineWiringStyle
	 *
	 * Populates Params with per-connection colour and thickness derived from
	 * the active UltraWire profile and the pin types involved.
	 */
	virtual void DetermineWiringStyle(
		UEdGraphPin* OutputPin,
		UEdGraphPin* InputPin,
		FConnectionParams& Params) override;

	// -------------------------------------------------------------------------
	// Post-draw pass
	// -------------------------------------------------------------------------

	/**
	 * DrawCrossings
	 *
	 * Must be called once after all DrawConnection() calls for a frame have
	 * completed.  Runs crossing detection on the accumulated WirePaths and
	 * renders a crossing symbol at each intersection according to the active
	 * CrossingStyle setting.
	 */
	void DrawCrossings();

	// -------------------------------------------------------------------------
	// Accessors
	// -------------------------------------------------------------------------

	/** Returns the graph type this policy was created for. */
	EUltraWireGraphType GetGraphType() const { return GraphType; }

protected:
	// -------------------------------------------------------------------------
	// Internal helpers
	// -------------------------------------------------------------------------

	/**
	 * ComputeWirePath
	 *
	 * Dispatches to the appropriate geometry engine method based on the active
	 * wire style setting, then applies corner rounding and ribbon offset.
	 */
	TArray<FVector2D> ComputeWirePath(const FVector2D& Start, const FVector2D& End) const;

	/**
	 * DrawWireLines
	 *
	 * Draws the actual polyline segments for a wire path, applying Gap
	 * crossing masks if CrossingStyle == Gap.
	 */
	void DrawWireLines(
		int32 LayerID,
		const TArray<FVector2D>& WirePath,
		const FLinearColor& Color,
		float Thickness);

	/**
	 * GetActiveProfile
	 *
	 * Returns the UltraWireProfile for this policy's graph type.
	 */
	const FUltraWireProfile& GetActiveProfile() const;

	// -------------------------------------------------------------------------
	// Per-frame state
	// -------------------------------------------------------------------------

	/** Accumulated wire paths for crossing detection this frame. */
	TArray<TArray<FVector2D>> WirePaths;

	/** Colours of each wire stored in WirePaths, for crossing symbol colouring. */
	TArray<FLinearColor> WireColors;

	/** Which graph type this policy is drawing. */
	EUltraWireGraphType GraphType;

	/** Pointer to the graph object (may be nullptr for non-Blueprint graphs). */
	UEdGraph* GraphObj;

	/**
	 * Cached geometry of the graph panel widget.  Set once at construction via
	 * the SGraphPanel pointer and used throughout the lifetime of this policy
	 * instance (one frame).  We store a copy so subsystems can use it without
	 * needing a panel pointer.
	 */
	FGeometry CachedAllottedGeometry;

	/** Current time in seconds, captured once at construction for consistency. */
	double CurrentTime;
};
