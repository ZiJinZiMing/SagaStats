// Copyright 2026 StraySpark. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Math/Color.h"
#include "Containers/Map.h"
#include "Misc/Guid.h"

class UEdGraphPin;

/**
 * FUltraWireHeatmapBridge
 *
 * Singleton that aggregates Blueprint profiler statistics and exposes per-pin
 * "heat" values (normalized 0-1 execution frequency) for UltraWire's heatmap
 * wire visualization.
 *
 * Lifetime:
 *   Created by FUltraWireProfilerModule::StartupModule().
 *   Destroyed by FUltraWireProfilerModule::ShutdownModule().
 *   Callers must NOT delete the instance returned by Get().
 *
 * Thread safety:
 *   All public methods are intended to be called from the game/editor thread.
 *   Internal data structures are not protected by a lock; do not call from
 *   worker threads.
 *
 * Data source:
 *   When available the bridge subscribes to FBlueprintCoreDelegates profiling
 *   callbacks.  If the profiler is not present at compile time (UE build
 *   configurations that strip it) the bridge silently operates with an empty
 *   cache – callers will always receive 0.0 heat and no assertions will fire.
 */
class ULTRAWIREPROFILER_API FUltraWireHeatmapBridge
{
public:
	// -------------------------------------------------------------------------
	// Singleton access
	// -------------------------------------------------------------------------

	/**
	 * Returns the singleton instance.
	 * Will return nullptr if called before StartupModule() or after
	 * ShutdownModule().  Callers should guard with IsValid().
	 */
	static FUltraWireHeatmapBridge* Get();

	/** Returns true if the singleton has been created and is ready to use. */
	static bool IsValid();

	// -------------------------------------------------------------------------
	// Lifecycle (called by FUltraWireProfilerModule only)
	// -------------------------------------------------------------------------

	/** Creates the singleton.  Must be called exactly once, by the module. */
	static void Initialize();

	/** Destroys the singleton.  Must be called exactly once, by the module. */
	static void Shutdown();

	// -------------------------------------------------------------------------
	// Active state
	// -------------------------------------------------------------------------

	/**
	 * When false the bridge ignores all incoming profiler callbacks and
	 * GetHeatForPin() always returns 0.0.  Flipped by the module when it
	 * detects that Blueprint profiling has started or stopped.
	 */
	bool bIsActive = false;

	/** Activates the bridge and begins processing profiler data. */
	void Activate();

	/** Deactivates the bridge.  Existing cache entries are preserved. */
	void Deactivate();

	// -------------------------------------------------------------------------
	// Data ingestion
	// -------------------------------------------------------------------------

	/**
	 * Polls the current Blueprint profiler state and refreshes the cache.
	 * Called automatically by the module tick but can be invoked on demand.
	 *
	 * Internally this reads the most recent profiler event batch, extracts
	 * execution counts per pin GUID, normalises them to [0,1] against the
	 * maximum observed count, and writes the result into CachedHeatMap.
	 *
	 * If bIsActive is false this function returns immediately without touching
	 * the cache.
	 */
	void UpdateProfilingData();

	/**
	 * Clears all cached heat values.
	 * The bridge remains active; new data will be gathered on the next
	 * UpdateProfilingData() call.
	 */
	void ResetData();

	// -------------------------------------------------------------------------
	// Query API
	// -------------------------------------------------------------------------

	/**
	 * Returns the normalised heat value (0 = cold / never executed,
	 * 1 = hot / most-executed) for the given pin.
	 *
	 * Returns 0.0 if:
	 *   - The bridge is inactive (bIsActive == false)
	 *   - The pin has no GUID (PersistentGuid.IsValid() == false)
	 *   - The pin has not been observed in any profiler event batch
	 *
	 * @param Pin  The graph pin to query.  May be nullptr (returns 0.0).
	 * @return     Heat in [0, 1].
	 */
	float GetHeatForPin(const UEdGraphPin* Pin) const;

	/**
	 * Returns the heat value stored directly by GUID, bypassing the pin lookup.
	 * Useful when the pin object is no longer available but its GUID is known.
	 *
	 * @param PinGuid  The persistent GUID of the pin.
	 * @return         Heat in [0, 1], or 0.0 if not found.
	 */
	float GetHeatForGuid(const FGuid& PinGuid) const;

	/**
	 * Linearly interpolates between ColdColor and HotColor using Heat as the
	 * alpha.  Heat is clamped to [0, 1] before interpolation.
	 *
	 * @param Heat       Normalised execution frequency, 0-1.
	 * @param ColdColor  Color returned when Heat == 0.
	 * @param HotColor   Color returned when Heat == 1.
	 * @return           Interpolated linear color.
	 */
	static FLinearColor GetHeatColor(float Heat,
	                                 FLinearColor ColdColor,
	                                 FLinearColor HotColor);

	// -------------------------------------------------------------------------
	// Cache inspection (for tooling / tests)
	// -------------------------------------------------------------------------

	/** Returns the number of pins currently stored in the heat cache. */
	int32 GetCachedPinCount() const { return CachedHeatMap.Num(); }

	/** Read-only view of the raw heat cache. */
	const TMap<FGuid, float>& GetCachedHeatMap() const { return CachedHeatMap; }

private:
	// -------------------------------------------------------------------------
	// Construction / destruction (private – use Initialize / Shutdown)
	// -------------------------------------------------------------------------

	FUltraWireHeatmapBridge();
	~FUltraWireHeatmapBridge();

	// Non-copyable
	FUltraWireHeatmapBridge(const FUltraWireHeatmapBridge&) = delete;
	FUltraWireHeatmapBridge& operator=(const FUltraWireHeatmapBridge&) = delete;

	// -------------------------------------------------------------------------
	// Internal helpers
	// -------------------------------------------------------------------------

	/**
	 * Registers / unregisters FBlueprintCoreDelegates callbacks.
	 * No-ops on builds where the profiler is not available.
	 */
	void BindProfilerDelegates();
	void UnbindProfilerDelegates();

	/**
	 * Called by FBlueprintCoreDelegates when a Blueprint profiling session
	 * starts.  Sets bIsActive = true and resets the cache.
	 */
	void OnBlueprintProfilingEnabled();

	/**
	 * Called by FBlueprintCoreDelegates when a Blueprint profiling session
	 * ends.  Sets bIsActive = false.  Cached data is preserved so heatmap
	 * values remain visible after the session stops.
	 */
	void OnBlueprintProfilingDisabled();

	/**
	 * Normalises the raw execution-count map so that the pin with the highest
	 * count maps to 1.0 and all other pins are proportional to it.
	 * Writes directly into CachedHeatMap.
	 *
	 * @param RawCounts  Map from pin GUID to absolute execution count.
	 */
	void NormaliseAndCache(const TMap<FGuid, uint64>& RawCounts);

	// -------------------------------------------------------------------------
	// State
	// -------------------------------------------------------------------------

	/**
	 * Per-pin normalised heat values.
	 * Key:   Persistent pin GUID (UEdGraphPin::PersistentGuid).
	 * Value: Normalised heat in [0, 1].
	 */
	TMap<FGuid, float> CachedHeatMap;

	/**
	 * Raw execution counts accumulated since the last ResetData() call.
	 * Kept separately so we can re-normalise without losing historical totals.
	 * Key:   Persistent pin GUID.
	 * Value: Total execution count.
	 */
	TMap<FGuid, uint64> RawExecutionCounts;

	/** Maximum raw execution count across all pins.  Used for normalisation. */
	uint64 MaxObservedCount = 0;

	/** Delegate handles for FBlueprintCoreDelegates subscriptions. */
	FDelegateHandle ProfilerEnabledHandle;
	FDelegateHandle ProfilerDisabledHandle;

	// -------------------------------------------------------------------------
	// Singleton storage
	// -------------------------------------------------------------------------

	/** The one and only instance, owned entirely by this class. */
	static FUltraWireHeatmapBridge* Instance;
};
