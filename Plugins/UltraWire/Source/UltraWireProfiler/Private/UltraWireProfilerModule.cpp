// Copyright 2026 StraySpark. All Rights Reserved.

#include "UltraWireProfilerModule.h"
#include "UltraWireHeatmapBridge.h"

#include "Containers/Ticker.h"
#include "Logging/LogMacros.h"
#include "Modules/ModuleManager.h"

DEFINE_LOG_CATEGORY_STATIC(LogUltraWireProfiler, Log, All);

// ---------------------------------------------------------------------------
// IModuleInterface
// ---------------------------------------------------------------------------

void FUltraWireProfilerModule::StartupModule()
{
	UE_LOG(LogUltraWireProfiler, Log, TEXT("UltraWireProfiler: StartupModule"));

	// Create the singleton bridge.  All profiler-delegate subscriptions happen
	// inside Initialize() so we don't need to touch delegate machinery here.
	FUltraWireHeatmapBridge::Initialize();

	// Register a per-frame tick so we can drive incremental data refreshes
	// without requiring any game-thread polling from callers.
	TickHandle = FTSTicker::GetCoreTicker().AddTicker(
		FTickerDelegate::CreateRaw(this, &FUltraWireProfilerModule::OnTick),
		/*InDelay=*/0.25f   // Poll at ~4 Hz; fast enough for live heatmap, cheap enough not to matter.
	);

	UE_LOG(LogUltraWireProfiler, Log, TEXT("UltraWireProfiler: Heatmap bridge initialised, tick registered."));
}

void FUltraWireProfilerModule::ShutdownModule()
{
	UE_LOG(LogUltraWireProfiler, Log, TEXT("UltraWireProfiler: ShutdownModule"));

	// Remove the tick delegate first so it can't fire after the bridge is gone.
	if (TickHandle.IsValid())
	{
		FTSTicker::GetCoreTicker().RemoveTicker(TickHandle);
		TickHandle.Reset();
	}

	// Tear down the bridge (unregisters profiler delegates, frees memory).
	FUltraWireHeatmapBridge::Shutdown();
}

// ---------------------------------------------------------------------------
// Public API
// ---------------------------------------------------------------------------

bool FUltraWireProfilerModule::IsProfilingActive() const
{
	if (FUltraWireHeatmapBridge::IsValid())
	{
		return FUltraWireHeatmapBridge::Get()->bIsActive;
	}
	return false;
}

void FUltraWireProfilerModule::RequestDataRefresh()
{
	if (FUltraWireHeatmapBridge::IsValid())
	{
		FUltraWireHeatmapBridge::Get()->UpdateProfilingData();
	}
}

// ---------------------------------------------------------------------------
// Private
// ---------------------------------------------------------------------------

bool FUltraWireProfilerModule::OnTick(float DeltaTime)
{
	// Drive incremental data refresh on each tick interval.
	// The bridge itself guards against doing work when inactive.
	if (FUltraWireHeatmapBridge::IsValid())
	{
		FUltraWireHeatmapBridge::Get()->UpdateProfilingData();
	}

	// Return true to keep the ticker alive for the lifetime of this module.
	return true;
}

// ---------------------------------------------------------------------------
// Module registration
// ---------------------------------------------------------------------------

IMPLEMENT_MODULE(FUltraWireProfilerModule, UltraWireProfiler)
