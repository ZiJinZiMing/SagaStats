// Copyright 2026 StraySpark. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "EdGraphUtilities.h"
#include "UltraWireTypes.h"

class UEdGraph;
class UEdGraphSchema;
class FConnectionDrawingPolicy;
class FSlateWindowElementList;

/**
 * FUltraWirePinConnectionFactory
 *
 * Registers with FEdGraphUtilities so that the graph panel calls into
 * UltraWire instead of the built-in connection drawing implementation.
 *
 * One factory instance is created per graph type (Blueprint, Material, etc.)
 * so that the policy can be queried for its graph type without inspecting the
 * schema class every frame.  The module creates and registers all instances on
 * startup and unregisters them on shutdown.
 *
 * CreateConnectionPolicy() returns a new FUltraWireDrawingPolicy allocated
 * with operator new; the graph panel takes ownership and deletes it.
 *
 * Real UE5 CreateConnectionPolicy signature (from EdGraphUtilities.h):
 *   FConnectionDrawingPolicy* CreateConnectionPolicy(
 *       const UEdGraphSchema* Schema,
 *       int32 InBackLayerID, int32 InFrontLayerID,
 *       float ZoomFactor,
 *       const FSlateRect& InClippingRect,
 *       FSlateWindowElementList& InDrawElements,
 *       UEdGraph* InGraphObj) const;
 */
class ULTRAWIRERENDERER_API FUltraWirePinConnectionFactory : public FGraphPanelPinConnectionFactory
{
public:
	/**
	 * @param InGraphType  The graph type this factory is responsible for.
	 *                     Used by the policy to look up the correct settings
	 *                     profile and is exposed to the module for bookkeeping.
	 */
	explicit FUltraWirePinConnectionFactory(EUltraWireGraphType InGraphType);

	virtual ~FUltraWirePinConnectionFactory() override = default;

	// -------------------------------------------------------------------------
	// FGraphPanelPinConnectionFactory override
	// -------------------------------------------------------------------------

	/**
	 * CreateConnectionPolicy
	 *
	 * Returns a heap-allocated FUltraWireDrawingPolicy for every graph type
	 * we support.  Returns nullptr for graph types we do not handle so the
	 * engine falls back to its own default.
	 *
	 * The returned pointer is owned (and deleted) by the graph panel widget.
	 */
	virtual FConnectionDrawingPolicy* CreateConnectionPolicy(
		const UEdGraphSchema* Schema,
		int32 InBackLayerID,
		int32 InFrontLayerID,
		float ZoomFactor,
		const FSlateRect& InClippingRect,
		FSlateWindowElementList& InDrawElements,
		UEdGraph* InGraphObj) const override;

	// -------------------------------------------------------------------------
	// Accessors
	// -------------------------------------------------------------------------

	/** Returns the graph type this factory was constructed for. */
	EUltraWireGraphType GetGraphType() const { return GraphType; }

private:
	/**
	 * Maps a UEdGraphSchema class to an EUltraWireGraphType so the policy can
	 * look up the correct settings profile.
	 *
	 * @param Schema  The schema of the graph being rendered.
	 * @return        The matching EUltraWireGraphType, or EUltraWireGraphType::Other.
	 */
	static EUltraWireGraphType SchemaToGraphType(const UEdGraphSchema* Schema);

	/** The graph type this instance was created to handle. */
	EUltraWireGraphType GraphType;
};
