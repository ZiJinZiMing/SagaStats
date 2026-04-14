// Copyright 2026 StraySpark. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Styling/SlateStyle.h"
#include "UltraWireTypes.h"

struct FSlateBrush;
struct FUltraWireProfile;

/**
 * FUltraWireThemeEngine
 *
 * Applies and removes visual overrides to the Unreal graph editor's Slate
 * style set.  All modifications are reversible: ApplyTheme() snapshots the
 * original brushes and style properties before overwriting them, and
 * ResetTheme() writes those originals back.
 *
 * The engine works exclusively with named Slate style entries so that it
 * remains compatible with future engine versions that may change how graph
 * nodes are drawn.  The targeted style names are:
 *
 *   Graph.Node.Body              – The rounded rectangle behind a node.
 *   Graph.Node.TitleGloss        – Specular/gloss overlay on the title bar.
 *   Graph.Node.ColorSpill        – Color band that bleeds from the title.
 *   Graph.Node.TitleHighlight    – Lighter stripe at the top of the title.
 *   Graph.Node.ShadowSize        – Drop-shadow inset border brush.
 *   Graph.Node.Shadow            – Drop-shadow (no-margin) brush.
 *   Graph.Node.PinPoint          – Dot drawn at the pin tip.
 *   Graph.Node.PinDiffIndicator  – Diff-highlighting brush on pins.
 *   Graph.CommentBubble.Body     – Background of comment nodes.
 *   Graph.CommentBubble.Border   – Border of comment nodes.
 */
class ULTRAWIRETHEME_API FUltraWireThemeEngine
{
public:

	FUltraWireThemeEngine();
	~FUltraWireThemeEngine();

	// Non-copyable – the engine owns heap-allocated FSlateBrush snapshots.
	FUltraWireThemeEngine(const FUltraWireThemeEngine&) = delete;
	FUltraWireThemeEngine& operator=(const FUltraWireThemeEngine&) = delete;

	/**
	 * Reads the supplied profile and pushes the corresponding values into
	 * FAppStyle's graph node style set.  Safe to call repeatedly; each call
	 * snapshots state relative to the last Reset (i.e., always the originals).
	 */
	void ApplyTheme(const FUltraWireProfile& Profile);

	/**
	 * Writes the original brushes and values back into FAppStyle, returning
	 * it to the state it was in before the first ApplyTheme call.
	 */
	void ResetTheme();

	/** True when ApplyTheme has been called at least once without a subsequent ResetTheme. */
	bool IsThemeApplied() const { return bThemeApplied; }

private:

	// ------------------------------------------------------------------
	// Helper types
	// ------------------------------------------------------------------

	/** A saved copy of one named brush entry in the style set. */
	struct FSavedBrush
	{
		FName StyleName;
		FSlateBrush OriginalBrush;
	};

	// ------------------------------------------------------------------
	// Internal helpers
	// ------------------------------------------------------------------

	/**
	 * Snapshots the current value of a named brush from FAppStyle if we have
	 * not already saved it.  This is called once per name; subsequent calls
	 * for the same name are no-ops so that multiple ApplyTheme calls do not
	 * overwrite our saved originals with already-modified values.
	 */
	void SnapshotBrushIfNeeded(const FName& StyleName);

	/**
	 * Creates an FSlateBrush whose tint and draw type reflect the given
	 * parameters, copied from an existing brush so that the image resource
	 * stays the same.
	 */
	static FSlateBrush MakeTintedBrush(const FSlateBrush& Source,
	                                   const FLinearColor& Tint,
	                                   float OpacityScale = 1.0f);

	/**
	 * Writes a modified brush into the FAppStyle style set.
	 * FSlateStyleSet exposes Set() as a public method only inside the module
	 * that owns it; we work around this by casting through the ISlateStyle
	 * interface's const-correct accessor – see implementation for details.
	 */
	void SetStyleBrush(const FName& StyleName, const FSlateBrush& Brush);

	/** Applies pin-shape overrides derived from EUltraWirePinShape. */
	void ApplyPinShapeOverrides(EUltraWirePinShape PinShape);

	/** Applies comment-box style overrides. */
	void ApplyCommentBoxOverrides(const FUltraWireProfile& Profile);

	/** Applies node body opacity override. */
	void ApplyNodeBodyOverrides(const FUltraWireProfile& Profile);

	/** Applies node header color tint overrides. */
	void ApplyNodeHeaderOverrides(const FUltraWireProfile& Profile);

	// ------------------------------------------------------------------
	// State
	// ------------------------------------------------------------------

	/** True once ApplyTheme has been called and snapshots exist. */
	bool bThemeApplied = false;

	/** Saved original brush data, keyed by style name. */
	TMap<FName, FSavedBrush> SavedBrushes;

	/** Saved original color tint for the node title color spill. */
	FLinearColor SavedColorSpillTint;
	bool bColorSpillTintSaved = false;
};
