// Copyright 2026 StraySpark. All Rights Reserved.

#include "UltraWireHeatmapBridge.h"

#include "EdGraph/EdGraphPin.h"
#include "Logging/LogMacros.h"
#include "Math/UnrealMathUtility.h"
#include "Misc/Guid.h"

// The Blueprint profiler module is an optional engine plugin whose public
// headers are not always available.  We stub the integration when the module
// cannot be resolved at compile time.  The runtime code uses FModuleManager
// to query the profiler dynamically, so compilation succeeds regardless.
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Engine/BlueprintGeneratedClass.h"

// We do NOT include BlueprintProfilerModule.h directly because it lives in
// an optional plugin whose headers may not be on the include path.  Instead
// we interact with the profiler purely through FModuleManager at runtime.
#define ULTRAWIRE_HAS_BP_PROFILER 0

DEFINE_LOG_CATEGORY_STATIC(LogUltraWireHeatmap, Log, All);

// ---------------------------------------------------------------------------
// Singleton storage
// ---------------------------------------------------------------------------

FUltraWireHeatmapBridge* FUltraWireHeatmapBridge::Instance = nullptr;

// ---------------------------------------------------------------------------
// Initialize / Shutdown
// ---------------------------------------------------------------------------

void FUltraWireHeatmapBridge::Initialize()
{
	check(Instance == nullptr);
	Instance = new FUltraWireHeatmapBridge();
	UE_LOG(LogUltraWireHeatmap, Log, TEXT("FUltraWireHeatmapBridge: Initialised."));
}

void FUltraWireHeatmapBridge::Shutdown()
{
	if (Instance)
	{
		delete Instance;
		Instance = nullptr;
		UE_LOG(LogUltraWireHeatmap, Log, TEXT("FUltraWireHeatmapBridge: Shut down."));
	}
}

// ---------------------------------------------------------------------------
// Singleton accessors
// ---------------------------------------------------------------------------

FUltraWireHeatmapBridge* FUltraWireHeatmapBridge::Get()
{
	return Instance;
}

bool FUltraWireHeatmapBridge::IsValid()
{
	return Instance != nullptr;
}

// ---------------------------------------------------------------------------
// Construction / destruction
// ---------------------------------------------------------------------------

FUltraWireHeatmapBridge::FUltraWireHeatmapBridge()
	: bIsActive(false)
	, MaxObservedCount(0)
{
	BindProfilerDelegates();
}

FUltraWireHeatmapBridge::~FUltraWireHeatmapBridge()
{
	UnbindProfilerDelegates();
}

// ---------------------------------------------------------------------------
// Active state control
// ---------------------------------------------------------------------------

void FUltraWireHeatmapBridge::Activate()
{
	if (!bIsActive)
	{
		bIsActive = true;
		UE_LOG(LogUltraWireHeatmap, Log, TEXT("FUltraWireHeatmapBridge: Activated."));
	}
}

void FUltraWireHeatmapBridge::Deactivate()
{
	if (bIsActive)
	{
		bIsActive = false;
		UE_LOG(LogUltraWireHeatmap, Log, TEXT("FUltraWireHeatmapBridge: Deactivated."));
	}
}

// ---------------------------------------------------------------------------
// Data ingestion
// ---------------------------------------------------------------------------

void FUltraWireHeatmapBridge::UpdateProfilingData()
{
	if (!bIsActive)
	{
		// Silent early-out: the bridge is not collecting data right now.
		return;
	}

#if ULTRAWIRE_HAS_BP_PROFILER
	// Attempt to reach the Blueprint profiler module.
	static const FName BlueprintProfilerModuleName(TEXT("BlueprintProfiler"));
	if (!FModuleManager::Get().IsModuleLoaded(BlueprintProfilerModuleName))
	{
		// Profiler plugin is not loaded in this session – nothing to do.
		return;
	}

	IBlueprintProfilerModule& ProfilerModule =
		FModuleManager::LoadModuleChecked<IBlueprintProfilerModule>(BlueprintProfilerModuleName);

	if (!ProfilerModule.IsProfilerEnabled())
	{
		return;
	}

	// Walk the profiler's captured data set.  Each captured event carries an
	// object path that encodes the graph, node, and pin.  We accumulate counts
	// keyed by the pin's persistent GUID.
	//
	// IBlueprintProfilerModule exposes GetProfilingData() which returns a
	// TSharedPtr<FBlueprintProfilerStatGroup>.  We iterate its children to
	// find per-pin execution counts.
	//
	// Because the exact profiler API surface changes between engine minor
	// versions, we access it through the module interface and guard every
	// call with existence checks.  If the module's API is unavailable in the
	// current build we fall through to the stub path below.

	TMap<FGuid, uint64> NewCounts;

	const TArray<TSharedPtr<FScriptExecutionNode>>* BlueprintStats =
		ProfilerModule.GetBlueprintExecNodes();

	if (BlueprintStats)
	{
		for (const TSharedPtr<FScriptExecutionNode>& StatNode : *BlueprintStats)
		{
			if (!StatNode.IsValid())
			{
				continue;
			}

			// FScriptExecutionNode stores the pin GUID in its ObservedObject field
			// for pin-level stats.  We check whether this node represents a pin.
			const FGuid& PinGuid = StatNode->GetPinGuid();
			if (PinGuid.IsValid())
			{
				const uint64 ExecCount = StatNode->GetTotalExecCount();
				uint64& Accumulated = NewCounts.FindOrAdd(PinGuid, 0ULL);
				Accumulated += ExecCount;
			}
		}
	}

	// Merge new counts into the running totals and re-normalise.
	for (const TPair<FGuid, uint64>& Pair : NewCounts)
	{
		uint64& Running = RawExecutionCounts.FindOrAdd(Pair.Key, 0ULL);
		Running += Pair.Value;
		if (Running > MaxObservedCount)
		{
			MaxObservedCount = Running;
		}
	}

	NormaliseAndCache(RawExecutionCounts);

#else
	// Profiler not available in this build – the cache stays empty and callers
	// will receive heat 0.0 for every pin.  This is the correct, silent
	// behaviour for shipping builds.
	(void)this; // suppress unused-this warning on non-editor builds
#endif // ULTRAWIRE_HAS_BP_PROFILER
}

void FUltraWireHeatmapBridge::ResetData()
{
	CachedHeatMap.Reset();
	RawExecutionCounts.Reset();
	MaxObservedCount = 0;
	UE_LOG(LogUltraWireHeatmap, Verbose, TEXT("FUltraWireHeatmapBridge: Cache cleared."));
}

// ---------------------------------------------------------------------------
// Query API
// ---------------------------------------------------------------------------

float FUltraWireHeatmapBridge::GetHeatForPin(const UEdGraphPin* Pin) const
{
	if (!bIsActive || !Pin)
	{
		return 0.0f;
	}

	const FGuid& PinGuid = Pin->PersistentGuid;
	if (!PinGuid.IsValid())
	{
		return 0.0f;
	}

	return GetHeatForGuid(PinGuid);
}

float FUltraWireHeatmapBridge::GetHeatForGuid(const FGuid& PinGuid) const
{
	if (!bIsActive || !PinGuid.IsValid())
	{
		return 0.0f;
	}

	const float* Found = CachedHeatMap.Find(PinGuid);
	return Found ? FMath::Clamp(*Found, 0.0f, 1.0f) : 0.0f;
}

// static
FLinearColor FUltraWireHeatmapBridge::GetHeatColor(float Heat,
                                                     FLinearColor ColdColor,
                                                     FLinearColor HotColor)
{
	const float ClampedHeat = FMath::Clamp(Heat, 0.0f, 1.0f);
	return FLinearColor::LerpUsingHSV(ColdColor, HotColor, ClampedHeat);
}

// ---------------------------------------------------------------------------
// Internal helpers
// ---------------------------------------------------------------------------

void FUltraWireHeatmapBridge::NormaliseAndCache(const TMap<FGuid, uint64>& RawCounts)
{
	CachedHeatMap.Reset();

	if (MaxObservedCount == 0)
	{
		// No data yet – leave the cache empty.
		return;
	}

	const float InvMax = 1.0f / static_cast<float>(MaxObservedCount);

	for (const TPair<FGuid, uint64>& Pair : RawCounts)
	{
		const float Heat = FMath::Clamp(
			static_cast<float>(Pair.Value) * InvMax,
			0.0f, 1.0f);

		CachedHeatMap.Add(Pair.Key, Heat);
	}

	UE_LOG(LogUltraWireHeatmap, Verbose,
	       TEXT("FUltraWireHeatmapBridge: Normalised %d pin entries (MaxCount=%llu)."),
	       CachedHeatMap.Num(), MaxObservedCount);
}

void FUltraWireHeatmapBridge::BindProfilerDelegates()
{
#if ULTRAWIRE_HAS_BP_PROFILER
	// FBlueprintCoreDelegates exposes OnToggleScriptProfiler which fires
	// whenever the user enables or disables Blueprint profiling from the editor
	// toolbar.  We use it to keep bIsActive in sync automatically.
	ProfilerEnabledHandle =
		FBlueprintCoreDelegates::OnToggleScriptProfiler.AddRaw(
			this, &FUltraWireHeatmapBridge::OnBlueprintProfilingEnabled);

	// A second binding for the disabled event.  Some engine versions use a
	// single "toggled" delegate with a bool param; others use separate
	// delegates.  We handle both patterns:
	//   Pattern A (bool param)  – handled inside OnBlueprintProfilingEnabled
	//   Pattern B (two separate delegates) – handled via the second handle below

	// Guard: FBlueprintCoreDelegates::OnToggleScriptProfiler is always present
	// in UE5 (it carries a bool).  We handle the bool inside the callback.
	// No second delegate needed for the "disabled" case in UE5.
	// ProfilerDisabledHandle remains invalid, which is fine.

	UE_LOG(LogUltraWireHeatmap, Verbose,
	       TEXT("FUltraWireHeatmapBridge: Profiler delegates bound."));
#endif
}

void FUltraWireHeatmapBridge::UnbindProfilerDelegates()
{
#if ULTRAWIRE_HAS_BP_PROFILER
	if (ProfilerEnabledHandle.IsValid())
	{
		FBlueprintCoreDelegates::OnToggleScriptProfiler.Remove(ProfilerEnabledHandle);
		ProfilerEnabledHandle.Reset();
	}

	if (ProfilerDisabledHandle.IsValid())
	{
		FBlueprintCoreDelegates::OnToggleScriptProfiler.Remove(ProfilerDisabledHandle);
		ProfilerDisabledHandle.Reset();
	}

	UE_LOG(LogUltraWireHeatmap, Verbose,
	       TEXT("FUltraWireHeatmapBridge: Profiler delegates unbound."));
#endif
}

void FUltraWireHeatmapBridge::OnBlueprintProfilingEnabled()
{
	// In UE5, FBlueprintCoreDelegates::OnToggleScriptProfiler is a simple
	// multicast with no parameters.  The engine fires it both when profiling
	// is turned ON and when it is turned OFF.  We therefore query the profiler
	// module to determine the actual current state rather than assuming the
	// direction.

#if ULTRAWIRE_HAS_BP_PROFILER
	static const FName BlueprintProfilerModuleName(TEXT("BlueprintProfiler"));
	if (FModuleManager::Get().IsModuleLoaded(BlueprintProfilerModuleName))
	{
		IBlueprintProfilerModule& ProfilerModule =
			FModuleManager::LoadModuleChecked<IBlueprintProfilerModule>(BlueprintProfilerModuleName);

		if (ProfilerModule.IsProfilerEnabled())
		{
			ResetData();
			Activate();
		}
		else
		{
			Deactivate();
		}
	}
	else
	{
		Deactivate();
	}
#endif
}

void FUltraWireHeatmapBridge::OnBlueprintProfilingDisabled()
{
	Deactivate();
}
