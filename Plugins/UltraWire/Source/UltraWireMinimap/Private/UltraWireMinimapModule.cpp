// Copyright 2026 StraySpark. All Rights Reserved.

#include "UltraWireMinimapModule.h"
#include "SUltraWireMinimap.h"
#include "UltraWireSettings.h"
#include "UltraWireTypes.h"

#include "Framework/Docking/TabManager.h"
#include "Widgets/Docking/SDockTab.h"
#include "Widgets/SOverlay.h"
#include "GraphEditor.h"
#include "SGraphPanel.h"
#include "Modules/ModuleManager.h"

#define LOCTEXT_NAMESPACE "FUltraWireMinimapModule"

// ---------------------------------------------------------------------------
// IModuleInterface
// ---------------------------------------------------------------------------

void FUltraWireMinimapModule::StartupModule()
{
	// Listen for tab focus changes so we can inject the minimap whenever a
	// new graph editor tab becomes visible.
	ActiveTabChangedHandle = FGlobalTabmanager::Get()->OnActiveTabChanged_Subscribe(
		FOnActiveTabChanged::FDelegate::CreateRaw(
			this, &FUltraWireMinimapModule::OnActiveTabChanged));

	// Subscribe to settings changes for hot-reload.
	if (UUltraWireSettings* Settings = UUltraWireSettings::Get())
	{
		SettingsChangedHandle = Settings->OnSettingsChanged.AddRaw(
			this, &FUltraWireMinimapModule::OnSettingsChanged);
	}
}

void FUltraWireMinimapModule::ShutdownModule()
{
	// Unsubscribe from the tab manager.
	FGlobalTabmanager::Get()->OnActiveTabChanged_Unsubscribe(ActiveTabChangedHandle);
	ActiveTabChangedHandle.Reset();

	// Unsubscribe from settings.
	if (UUltraWireSettings* Settings = UUltraWireSettings::Get())
	{
		Settings->OnSettingsChanged.Remove(SettingsChangedHandle);
	}
	SettingsChangedHandle.Reset();

	// Clear the weak panel list – no need to teardown overlays because
	// ShutdownModule is called as the editor closes.
	InjectedPanels.Empty();
}

// ---------------------------------------------------------------------------
// Static accessors
// ---------------------------------------------------------------------------

FUltraWireMinimapModule& FUltraWireMinimapModule::Get()
{
	return FModuleManager::LoadModuleChecked<FUltraWireMinimapModule>("UltraWireMinimap");
}

bool FUltraWireMinimapModule::IsAvailable()
{
	return FModuleManager::Get().IsModuleLoaded("UltraWireMinimap");
}

// ---------------------------------------------------------------------------
// Tab-change callback
// ---------------------------------------------------------------------------

void FUltraWireMinimapModule::OnActiveTabChanged(TSharedPtr<SDockTab> PreviouslyActive,
                                                  TSharedPtr<SDockTab> NewlyActive)
{
	if (!NewlyActive.IsValid())
	{
		return;
	}

	TryInjectMinimapIntoTab(NewlyActive.ToSharedRef());
}

// ---------------------------------------------------------------------------
// Settings hot-reload callback
// ---------------------------------------------------------------------------

void FUltraWireMinimapModule::OnSettingsChanged()
{
	// Purge stale weak pointers.
	InjectedPanels.RemoveAll([](const TWeakPtr<SGraphPanel>& Weak)
	{
		return !Weak.IsValid();
	});

	// For live panels the minimap widget reads settings on every Tick, so
	// we only need to handle the enabled/disabled transition here.  When the
	// minimap is newly enabled, panels that were previously visited won't have
	// an overlay yet – trigger a re-check of the currently active tab.
	TSharedPtr<SDockTab> ActiveTab = FGlobalTabmanager::Get()->GetActiveTab();
	if (ActiveTab.IsValid())
	{
		TryInjectMinimapIntoTab(ActiveTab.ToSharedRef());
	}
}

// ---------------------------------------------------------------------------
// Widget tree traversal
// ---------------------------------------------------------------------------

TSharedPtr<SGraphPanel> FUltraWireMinimapModule::FindGraphPanel(
	TSharedRef<SWidget> InWidget) const
{
	// Breadth-first search for SGraphPanel in the widget tree.
	TArray<TSharedRef<SWidget>> Queue;
	Queue.Add(InWidget);

	while (Queue.Num() > 0)
	{
		TSharedRef<SWidget> Current = Queue[0];
		Queue.RemoveAt(0, 1, false);

		// Check if this widget IS an SGraphPanel.
		if (Current->GetType() == TEXT("SGraphPanel"))
		{
			return StaticCastSharedRef<SGraphPanel>(Current);
		}

		// Recurse into children.
		FChildren* Children = Current->GetChildren();
		if (Children)
		{
			for (int32 i = 0; i < Children->Num(); ++i)
			{
				Queue.Add(Children->GetChildAt(i));
			}
		}
	}

	return nullptr;
}

// ---------------------------------------------------------------------------
// Minimap injection
// ---------------------------------------------------------------------------

void FUltraWireMinimapModule::TryInjectMinimapIntoTab(TSharedRef<SDockTab> Tab)
{
	const UUltraWireSettings* Settings = UUltraWireSettings::Get();
	if (!Settings || !Settings->bEnabled)
	{
		return;
	}

	// Walk the tab's content to find the graph panel.
	TSharedPtr<SWidget> TabContent = Tab->GetContent();
	if (!TabContent.IsValid())
	{
		return;
	}

	TSharedPtr<SGraphPanel> GraphPanel = FindGraphPanel(TabContent.ToSharedRef());
	if (!GraphPanel.IsValid())
	{
		return;
	}

	// Avoid double-injection: check whether this panel is already tracked.
	for (const TWeakPtr<SGraphPanel>& Weak : InjectedPanels)
	{
		if (Weak.Pin() == GraphPanel)
		{
			return;
		}
	}

	// The graph panel lives inside some ancestor SOverlay.  We need to find
	// that overlay and add a new slot with the minimap widget.  If the direct
	// parent of the graph panel is not an SOverlay we cannot inject without
	// engine modification, so we bail gracefully.
	//
	// In UE5 the graph panel hierarchy is typically:
	//   SBorder  (editor frame)
	//     SOverlay
	//       SGraphPanel
	//       ... (zoom display, comment controls, etc.)
	//
	// We locate the SOverlay by walking up via the registered parent.  Slate
	// does not expose a GetParent() API, so we search from the tab content
	// down, stopping one level above the graph panel.

	struct FOverlayFinder
	{
		TSharedPtr<SOverlay> FoundOverlay;

		bool Search(TSharedRef<SWidget> Widget, TSharedRef<SWidget> Target)
		{
			FChildren* Children = Widget->GetChildren();
			if (!Children) { return false; }

			for (int32 i = 0; i < Children->Num(); ++i)
			{
				TSharedRef<SWidget> Child = Children->GetChildAt(i);
				if (Child == Target)
				{
					// Widget is the parent; check if it is an SOverlay.
					if (Widget->GetType() == TEXT("SOverlay"))
					{
						FoundOverlay = StaticCastSharedRef<SOverlay>(Widget);
					}
					return true;
				}
				if (Search(Child, Target)) { return true; }
			}
			return false;
		}
	} Finder;

	Finder.Search(TabContent.ToSharedRef(), GraphPanel.ToSharedRef());

	if (!Finder.FoundOverlay.IsValid())
	{
		UE_LOG(LogTemp, Verbose,
			TEXT("UltraWire Minimap: Could not locate SOverlay parent for SGraphPanel in this tab – skipping injection."));
		return;
	}

	// Determine corner alignment for the minimap based on settings.
	const FUltraWireProfile& Profile = Settings->DefaultProfile;

	EHorizontalAlignment HAlign = EHorizontalAlignment::HAlign_Right;
	EVerticalAlignment   VAlign = EVerticalAlignment::VAlign_Bottom;

	switch (Profile.MinimapPosition)
	{
	case EUltraWireMinimapPosition::TopLeft:
		HAlign = HAlign_Left;  VAlign = VAlign_Top;    break;
	case EUltraWireMinimapPosition::TopRight:
		HAlign = HAlign_Right; VAlign = VAlign_Top;    break;
	case EUltraWireMinimapPosition::BottomLeft:
		HAlign = HAlign_Left;  VAlign = VAlign_Bottom; break;
	case EUltraWireMinimapPosition::BottomRight:
		HAlign = HAlign_Right; VAlign = VAlign_Bottom; break;
	}

	// Padding from the edge so the minimap does not overlap scrollbars.
	const FMargin EdgePadding(8.0f);

	// Build and inject the minimap widget.
	TSharedRef<SUltraWireMinimap> MinimapWidget =
		SNew(SUltraWireMinimap)
		.GraphPanel(GraphPanel);

	Finder.FoundOverlay->AddSlot()
		.HAlign(HAlign)
		.VAlign(VAlign)
		.Padding(EdgePadding)
		[
			MinimapWidget
		];

	// Track this panel so we don't inject again.
	InjectedPanels.Add(GraphPanel);

	UE_LOG(LogTemp, Verbose,
		TEXT("UltraWire Minimap: Injected minimap overlay into graph editor tab."));
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FUltraWireMinimapModule, UltraWireMinimap)
