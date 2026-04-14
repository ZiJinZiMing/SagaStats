// Copyright 2026 StraySpark. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "IDetailCustomization.h"
#include "UltraWireTypes.h"

class IDetailLayoutBuilder;
class IDetailCategoryBuilder;
class IPropertyHandle;
class SWidget;

/**
 * FUltraWireSettingsCustomization
 *
 * Custom IDetailCustomization for UUltraWireSettings.
 *
 * Builds the settings UI displayed in:
 *   Edit > Project Settings > Plugins > UltraWire
 *
 * Layout overview
 * ---------------
 *   [General]          - Master enable toggle (default property)
 *   [Profile]          - Profile selector dropdown + import / export buttons
 *   [Built-in Presets] - "Apply Built-in Preset" combo with 5 factory presets
 *   [Default Profile]  - Default wire profile properties
 *   [Graph Profiles]   - Per-graph-type profile assignment table
 *
 * All Slate widgets that reference editor state are allocated on the heap and
 * kept alive by the SSharedPtr returned from CustomizeDetails.  The layout
 * builder is held only for the duration of CustomizeDetails() and must not be
 * cached; instead we capture IPropertyHandle references which remain valid
 * until the next layout rebuild.
 */
class ULTRAWIRESETTINGS_API FUltraWireSettingsCustomization : public IDetailCustomization
{
public:
	// -------------------------------------------------------------------------
	// IDetailCustomization factory
	// -------------------------------------------------------------------------

	/** Creates a new instance.  Used as the factory delegate by the module. */
	static TSharedRef<IDetailCustomization> MakeInstance();

	// -------------------------------------------------------------------------
	// IDetailCustomization overrides
	// -------------------------------------------------------------------------

	/**
	 * Entry point called by the PropertyEditor whenever a UUltraWireSettings
	 * object is selected for display.  Builds the full custom UI by adding
	 * rows, widgets, and custom category builders to DetailBuilder.
	 */
	virtual void CustomizeDetails(IDetailLayoutBuilder& DetailBuilder) override;

private:
	// -------------------------------------------------------------------------
	// Category builders
	// -------------------------------------------------------------------------

	/**
	 * Builds the "Profile" category containing:
	 *   - Saved-preset selector dropdown
	 *   - [Import Preset] and [Export Preset] action buttons
	 */
	void BuildProfileCategory(IDetailLayoutBuilder& DetailBuilder);

	/**
	 * Builds the "Built-in Presets" category containing a single combo box
	 * that lists the five factory presets.  Selecting a preset and pressing
	 * "Apply" copies it into the active profile.
	 */
	void BuildBuiltinPresetsCategory(IDetailLayoutBuilder& DetailBuilder);

	/**
	 * Builds the "Graph Profiles" category that lets users assign a saved
	 * preset (or the default profile) to each EUltraWireGraphType value.
	 */
	void BuildGraphProfilesCategory(IDetailLayoutBuilder& DetailBuilder);

	// -------------------------------------------------------------------------
	// Preset combo helpers
	// -------------------------------------------------------------------------

	/** Populates SavedPresetNames from the current UUltraWireSettings. */
	void RefreshSavedPresetList();

	/** Returns the widget to display for the currently-selected saved preset. */
	TSharedRef<SWidget> MakeSavedPresetComboEntry(TSharedPtr<FString> Item);
	FText                GetSavedPresetComboText() const;
	void                 OnSavedPresetSelected(TSharedPtr<FString> Item, ESelectInfo::Type SelectInfo);

	/** Returns the widget to display for a built-in preset entry. */
	TSharedRef<SWidget> MakeBuiltinPresetComboEntry(TSharedPtr<FString> Item);
	FText                GetBuiltinPresetComboText() const;
	void                 OnBuiltinPresetSelected(TSharedPtr<FString> Item, ESelectInfo::Type SelectInfo);

	// -------------------------------------------------------------------------
	// Button handlers
	// -------------------------------------------------------------------------

	/** Opens a save-file dialog and exports the currently-selected preset. */
	FReply OnExportPresetClicked();

	/** Opens an open-file dialog and imports a preset JSON file. */
	FReply OnImportPresetClicked();

	/** Applies the currently-selected built-in preset to the settings. */
	FReply OnApplyBuiltinPresetClicked();

	// -------------------------------------------------------------------------
	// Graph profile assignment helpers
	// -------------------------------------------------------------------------

	/**
	 * Builds one row for a given graph type inside the Graph Profiles category.
	 * Each row contains:
	 *   - A label with the graph type name
	 *   - A preset selector combo (same list as the main profile selector)
	 */
	void BuildGraphTypeRow(IDetailCategoryBuilder& Category, EUltraWireGraphType GraphType);

	TSharedRef<SWidget> MakeGraphPresetComboEntry(TSharedPtr<FString> Item);
	void                OnGraphPresetSelected(TSharedPtr<FString> Item,
	                                          ESelectInfo::Type SelectInfo,
	                                          EUltraWireGraphType GraphType);
	FText               GetGraphPresetComboText(EUltraWireGraphType GraphType) const;

	// -------------------------------------------------------------------------
	// Built-in preset definitions
	// -------------------------------------------------------------------------

	struct FBuiltinPreset
	{
		FString Name;
		FString Description;
	};

	/** Returns the five hard-coded factory presets. */
	static TArray<FBuiltinPreset> GetBuiltinPresets();

	/**
	 * Returns the FUltraWireProfile for the named built-in preset.
	 * Returns false if the name is not recognised.
	 */
	static bool GetBuiltinPresetProfile(const FString& PresetName,
	                                    FUltraWireProfile& OutProfile);

	// -------------------------------------------------------------------------
	// State
	// -------------------------------------------------------------------------

	/** Cached list of saved preset names used to populate combo boxes. */
	TArray<TSharedPtr<FString>> SavedPresetNames;

	/** Cached list of built-in preset names. */
	TArray<TSharedPtr<FString>> BuiltinPresetNames;

	/** The name of the preset currently selected in the saved-preset combo. */
	TSharedPtr<FString> SelectedSavedPreset;

	/** The name of the preset currently selected in the built-in combo. */
	TSharedPtr<FString> SelectedBuiltinPreset;

	/**
	 * Per-graph-type currently selected preset name.
	 * Used to drive the combo text without re-querying the settings object.
	 */
	TMap<EUltraWireGraphType, TSharedPtr<FString>> SelectedGraphPresets;

	/** Raw pointer to the layout builder – valid only during CustomizeDetails and its callbacks. */
	IDetailLayoutBuilder* CachedDetailBuilderPtr = nullptr;
};
