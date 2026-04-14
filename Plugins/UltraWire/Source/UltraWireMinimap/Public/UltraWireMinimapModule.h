// Copyright 2026 StraySpark. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

class SGraphPanel;
class SDockTab;
class FTabManager;

/**
 * FUltraWireMinimapModule
 *
 * Manages the lifecycle of the minimap overlay inside every Blueprint /
 * Material / Niagara graph editor tab.
 *
 * Registration strategy
 * ---------------------
 * UE5 graph editors host their graph panel inside an SDockTab via a
 * FWorkflowTabFactory or similar mechanism.  Rather than patching the factory
 * (which would require engine source changes), this module hooks into the
 * global FGlobalTabmanager::OnActiveTabChanged delegate and the
 * FAssetEditorManager::OnAssetEditorOpened delegate to detect when a new
 * graph tab becomes visible.  It then walks the widget tree of that tab to
 * locate the SGraphPanel and injects a minimap overlay using an SOverlay.
 *
 * Overlay injection
 * -----------------
 * Injecting a child widget into an already-constructed Slate tree requires
 * the target container to support dynamic children.  Graph editor panels are
 * wrapped in an SOverlay in UE5, making it straightforward to add a new slot
 * at the top.  SUltraWireMinimap is constructed with a weak pointer to the
 * discovered SGraphPanel so it can read node positions without ownership.
 *
 * Settings hot-reload
 * -------------------
 * When UUltraWireSettings::OnSettingsChanged fires, the module rebuilds any
 * live minimap widgets so that position, size, and opacity changes are
 * reflected immediately.
 */
class ULTRAWIREMINIMAP_API FUltraWireMinimapModule : public IModuleInterface
{
public:

	// IModuleInterface
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

	static FUltraWireMinimapModule& Get();
	static bool IsAvailable();

private:

	// ------------------------------------------------------------------
	// Internal helpers
	// ------------------------------------------------------------------

	/**
	 * Scans the widget tree rooted at InWidget for the first SGraphPanel
	 * descendant and returns it.  Returns an invalid shared pointer when no
	 * graph panel is found.
	 */
	TSharedPtr<SGraphPanel> FindGraphPanel(TSharedRef<SWidget> InWidget) const;

	/**
	 * Attempts to inject a minimap overlay into the graph panel of the
	 * supplied tab.  Safe to call on non-graph tabs; it will return without
	 * modifying the widget tree.
	 */
	void TryInjectMinimapIntoTab(TSharedRef<SDockTab> Tab);

	/** Called when any dock tab becomes active. */
	void OnActiveTabChanged(TSharedPtr<SDockTab> PreviouslyActive,
	                        TSharedPtr<SDockTab> NewlyActive);

	/** Called when UUltraWireSettings changes. */
	void OnSettingsChanged();

	// ------------------------------------------------------------------
	// State
	// ------------------------------------------------------------------

	/** Handle for the active-tab-changed subscription. */
	FDelegateHandle ActiveTabChangedHandle;

	/** Handle for the settings-changed subscription. */
	FDelegateHandle SettingsChangedHandle;

	/**
	 * Weak references to every live SGraphPanel that has received a minimap
	 * overlay.  Used when settings change to rebuild overlays in-place.
	 */
	TArray<TWeakPtr<SGraphPanel>> InjectedPanels;
};
