// Copyright 2026 StraySpark. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleInterface.h"
#include "Modules/ModuleManager.h"

/**
 * UltraWireProfiler Module
 *
 * Bridges Blueprint profiler data into UltraWire's heatmap visualization system.
 * This module is responsible for:
 *   - Starting up the FUltraWireHeatmapBridge singleton
 *   - Hooking into Unreal's Blueprint profiler delegates on module load
 *   - Tearing down the bridge and releasing delegate bindings on module unload
 *
 * The module is editor-only and must only be loaded in WITH_EDITOR builds.
 * Game code must never take a hard dependency on this module.
 */
class ULTRAWIREPROFILER_API FUltraWireProfilerModule : public IModuleInterface
{
public:
	/** IModuleInterface overrides */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

	/**
	 * Convenience accessor.
	 * Safe to call from any editor thread after the module has been loaded.
	 */
	static FUltraWireProfilerModule& Get()
	{
		return FModuleManager::LoadModuleChecked<FUltraWireProfilerModule>("UltraWireProfiler");
	}

	/** Returns true if the module is currently loaded. */
	static bool IsAvailable()
	{
		return FModuleManager::Get().IsModuleLoaded("UltraWireProfiler");
	}

	/**
	 * Returns true while Blueprint profiling is active and the heatmap bridge is
	 * collecting data.  Useful for UI to conditionally display heatmap controls.
	 */
	bool IsProfilingActive() const;

	/**
	 * Manually trigger a data refresh from whatever profiling backend is
	 * currently active.  Normally this is called automatically by the bridge's
	 * tick delegate, but tools can call it on demand (e.g. when the user opens
	 * the heatmap overlay panel).
	 */
	void RequestDataRefresh();

private:
	/** Handle returned by FTicker so we can unregister our tick delegate. */
	FTSTicker::FDelegateHandle TickHandle;

	/**
	 * Called once per editor tick while the profiler module is loaded.
	 * Drives incremental data updates from the profiling backend.
	 *
	 * @param DeltaTime Seconds elapsed since the last tick.
	 * @return true to keep ticking; false to stop.
	 */
	bool OnTick(float DeltaTime);
};
