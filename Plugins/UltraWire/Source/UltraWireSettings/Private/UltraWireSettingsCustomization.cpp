// Copyright 2026 StraySpark. All Rights Reserved.

#include "UltraWireSettingsCustomization.h"
#include "UltraWireSettings.h"
#include "UltraWireTypes.h"

// Detail layout
#include "DetailLayoutBuilder.h"
#include "DetailCategoryBuilder.h"
#include "DetailWidgetRow.h"
#include "IDetailGroup.h"

// Slate
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SComboBox.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/SBoxPanel.h"
#include "Styling/AppStyle.h"

// Platform dialog
#include "DesktopPlatformModule.h"
#include "IDesktopPlatform.h"
#include "Framework/Application/SlateApplication.h"

// Reflection / property
#include "PropertyHandle.h"

// Enum utilities
#include "UObject/EnumProperty.h"

#include "Logging/LogMacros.h"

DEFINE_LOG_CATEGORY_STATIC(LogUltraWireSettingsUI, Log, All);

// ---------------------------------------------------------------------------
// Built-in preset definitions
// ---------------------------------------------------------------------------
// Five factory presets that represent common visual design philosophies.
// The actual profile data is defined in GetBuiltinPresetProfile() below.

namespace UltraWireBuiltinPresets
{
	static const FString Clean    = TEXT("Clean & Minimal");
	static const FString Neon     = TEXT("Neon Glow");
	static const FString Blueprint= TEXT("Blueprint Classic");
	static const FString Circuit  = TEXT("Circuit Board");
	static const FString Retro    = TEXT("Retro Terminal");
}

// ---------------------------------------------------------------------------
// MakeInstance
// ---------------------------------------------------------------------------

TSharedRef<IDetailCustomization> FUltraWireSettingsCustomization::MakeInstance()
{
	return MakeShareable(new FUltraWireSettingsCustomization());
}

// ---------------------------------------------------------------------------
// CustomizeDetails
// ---------------------------------------------------------------------------

void FUltraWireSettingsCustomization::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder)
{
	// Stash a raw pointer for use in callbacks that need to force a refresh.
	CachedDetailBuilderPtr = &DetailBuilder;

	// Pre-populate combo lists.
	RefreshSavedPresetList();

	BuiltinPresetNames.Reset();
	for (const FBuiltinPreset& BP : GetBuiltinPresets())
	{
		BuiltinPresetNames.Add(MakeShareable(new FString(BP.Name)));
	}

	// Ensure "none selected" default for the built-in combo.
	if (!SelectedBuiltinPreset.IsValid() && BuiltinPresetNames.Num() > 0)
	{
		SelectedBuiltinPreset = BuiltinPresetNames[0];
	}

	// Order in which categories appear in the panel.
	// DetailBuilder assigns display order by the sequence of EditCategory calls.

	// 1. General – keep default property display for bEnabled.
	IDetailCategoryBuilder& GeneralCategory = DetailBuilder.EditCategory(
		"General",
		FText::FromString(TEXT("General")),
		ECategoryPriority::Important);
	(void)GeneralCategory; // default property display handled automatically

	// 2. Profile selector + import / export.
	BuildProfileCategory(DetailBuilder);

	// 3. Built-in presets.
	BuildBuiltinPresetsCategory(DetailBuilder);

	// 4. Default profile – keep default property rows.
	DetailBuilder.EditCategory("Default Profile");

	// 5. Graph type profile assignment.
	BuildGraphProfilesCategory(DetailBuilder);

	// 6. Presets storage – keep default rows so saved presets are editable.
	DetailBuilder.EditCategory("Presets");
}

// ---------------------------------------------------------------------------
// Category: Profile
// ---------------------------------------------------------------------------

void FUltraWireSettingsCustomization::BuildProfileCategory(IDetailLayoutBuilder& DetailBuilder)
{
	IDetailCategoryBuilder& Category = DetailBuilder.EditCategory(
		"Profile",
		FText::FromString(TEXT("Profile Management")),
		ECategoryPriority::Important);

	// --- Saved Preset Selector ---
	Category.AddCustomRow(FText::FromString(TEXT("Active Preset")))
		.NameContent()
		[
			SNew(STextBlock)
			.Text(FText::FromString(TEXT("Active Preset")))
			.Font(DetailBuilder.GetDetailFont())
		]
		.ValueContent()
		.MinDesiredWidth(200.0f)
		[
			SNew(SComboBox<TSharedPtr<FString>>)
			.OptionsSource(&SavedPresetNames)
			.OnGenerateWidget_Raw(this, &FUltraWireSettingsCustomization::MakeSavedPresetComboEntry)
			.OnSelectionChanged_Raw(this, &FUltraWireSettingsCustomization::OnSavedPresetSelected)
			.Content()
			[
				SNew(STextBlock)
				.Text_Raw(this, &FUltraWireSettingsCustomization::GetSavedPresetComboText)
				.Font(DetailBuilder.GetDetailFont())
			]
		];

	// --- Import / Export Buttons ---
	Category.AddCustomRow(FText::FromString(TEXT("Import Export")))
		.WholeRowContent()
		[
			SNew(SHorizontalBox)

			// [Import Preset]
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.Padding(0.0f, 0.0f, 8.0f, 0.0f)
			[
				SNew(SButton)
				.Text(FText::FromString(TEXT("Import Preset...")))
				.ToolTipText(FText::FromString(TEXT("Import a UltraWire preset from a JSON file.")))
				.OnClicked_Raw(this, &FUltraWireSettingsCustomization::OnImportPresetClicked)
			]

			// [Export Preset]
			+ SHorizontalBox::Slot()
			.AutoWidth()
			[
				SNew(SButton)
				.Text(FText::FromString(TEXT("Export Preset...")))
				.ToolTipText(FText::FromString(TEXT("Export the selected preset to a JSON file.")))
				.OnClicked_Raw(this, &FUltraWireSettingsCustomization::OnExportPresetClicked)
			]
		];
}

// ---------------------------------------------------------------------------
// Category: Built-in Presets
// ---------------------------------------------------------------------------

void FUltraWireSettingsCustomization::BuildBuiltinPresetsCategory(IDetailLayoutBuilder& DetailBuilder)
{
	IDetailCategoryBuilder& Category = DetailBuilder.EditCategory(
		"BuiltinPresets",
		FText::FromString(TEXT("Built-in Presets")),
		ECategoryPriority::Default);

	// Description row.
	Category.AddCustomRow(FText::FromString(TEXT("Description")))
		.WholeRowContent()
		[
			SNew(STextBlock)
			.Text(FText::FromString(
				TEXT("Select a built-in preset and press \"Apply\" to overwrite the Default Profile.")))
			.AutoWrapText(true)
			.Font(DetailBuilder.GetDetailFont())
		];

	// Preset selector + Apply button on one row.
	Category.AddCustomRow(FText::FromString(TEXT("Apply Built-in Preset")))
		.NameContent()
		[
			SNew(STextBlock)
			.Text(FText::FromString(TEXT("Preset")))
			.Font(DetailBuilder.GetDetailFont())
		]
		.ValueContent()
		.MinDesiredWidth(350.0f)
		[
			SNew(SHorizontalBox)

			// Combo
			+ SHorizontalBox::Slot()
			.FillWidth(1.0f)
			.Padding(0.0f, 0.0f, 8.0f, 0.0f)
			[
				SNew(SComboBox<TSharedPtr<FString>>)
				.OptionsSource(&BuiltinPresetNames)
				.OnGenerateWidget_Raw(this, &FUltraWireSettingsCustomization::MakeBuiltinPresetComboEntry)
				.OnSelectionChanged_Raw(this, &FUltraWireSettingsCustomization::OnBuiltinPresetSelected)
				.Content()
				[
					SNew(STextBlock)
					.Text_Raw(this, &FUltraWireSettingsCustomization::GetBuiltinPresetComboText)
					.Font(DetailBuilder.GetDetailFont())
				]
			]

			// Apply button
			+ SHorizontalBox::Slot()
			.AutoWidth()
			[
				SNew(SButton)
				.Text(FText::FromString(TEXT("Apply")))
				.ToolTipText(FText::FromString(TEXT("Copy this preset's settings into the Default Profile.")))
				.OnClicked_Raw(this, &FUltraWireSettingsCustomization::OnApplyBuiltinPresetClicked)
			]
		];
}

// ---------------------------------------------------------------------------
// Category: Graph Profiles
// ---------------------------------------------------------------------------

void FUltraWireSettingsCustomization::BuildGraphProfilesCategory(IDetailLayoutBuilder& DetailBuilder)
{
	// Hide the raw TMap property – we provide a more readable per-row layout.
	TSharedRef<IPropertyHandle> GraphProfilesHandle =
		DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UUltraWireSettings, GraphProfileOverrides));
	DetailBuilder.HideProperty(GraphProfilesHandle);

	IDetailCategoryBuilder& Category = DetailBuilder.EditCategory(
		"Graph Profiles",
		FText::FromString(TEXT("Graph Type Profiles")),
		ECategoryPriority::Default);

	Category.AddCustomRow(FText::FromString(TEXT("Info")))
		.WholeRowContent()
		[
			SNew(STextBlock)
			.Text(FText::FromString(
				TEXT("Assign a saved preset to each graph type. "
				     "Leave empty to use the Default Profile.")))
			.AutoWrapText(true)
			.Font(DetailBuilder.GetDetailFont())
		];

	// One row per supported graph type.
	const TArray<EUltraWireGraphType> GraphTypes = {
		EUltraWireGraphType::Blueprint,
		EUltraWireGraphType::Material,
		EUltraWireGraphType::Niagara,
		EUltraWireGraphType::AnimationBlueprint,
		EUltraWireGraphType::BehaviorTree,
		EUltraWireGraphType::ControlRig,
		EUltraWireGraphType::PCG,
		EUltraWireGraphType::SoundCue,
		EUltraWireGraphType::MetaSound,
		EUltraWireGraphType::EnvironmentQuery,
		EUltraWireGraphType::GameplayAbility,
		EUltraWireGraphType::Other,
	};

	for (EUltraWireGraphType GraphType : GraphTypes)
	{
		BuildGraphTypeRow(Category, GraphType);
	}
}

void FUltraWireSettingsCustomization::BuildGraphTypeRow(
	IDetailCategoryBuilder& Category,
	EUltraWireGraphType GraphType)
{
	// Fetch the display name from the UEnum metadata.
	const UEnum* GraphTypeEnum = StaticEnum<EUltraWireGraphType>();
	const FString TypeName = GraphTypeEnum
		? GraphTypeEnum->GetDisplayNameTextByValue(static_cast<int64>(GraphType)).ToString()
		: FString::Printf(TEXT("Type %d"), static_cast<int32>(GraphType));

	// Build a local source list that prepends "(Default)" so the user can
	// clear a per-type override.
	TArray<TSharedPtr<FString>> LocalSource;
	LocalSource.Add(MakeShareable(new FString(TEXT("(Default)"))));
	for (const TSharedPtr<FString>& Name : SavedPresetNames)
	{
		LocalSource.Add(Name);
	}

	// Initialise selection state if not already present.
	if (!SelectedGraphPresets.Contains(GraphType))
	{
		SelectedGraphPresets.Add(GraphType, LocalSource[0]);
	}

	Category.AddCustomRow(FText::FromString(TypeName))
		.NameContent()
		[
			SNew(STextBlock)
			.Text(FText::FromString(TypeName))
		]
		.ValueContent()
		.MinDesiredWidth(200.0f)
		[
			SNew(SComboBox<TSharedPtr<FString>>)
			.OptionsSource(&SavedPresetNames) // uses the module-level list
			.OnGenerateWidget_Raw(this, &FUltraWireSettingsCustomization::MakeGraphPresetComboEntry)
			.OnSelectionChanged_Raw(
				this,
				&FUltraWireSettingsCustomization::OnGraphPresetSelected,
				GraphType)
			.Content()
			[
				SNew(STextBlock)
				.Text_Raw(this, &FUltraWireSettingsCustomization::GetGraphPresetComboText, GraphType)
			]
		];
}

// ---------------------------------------------------------------------------
// Saved preset combo helpers
// ---------------------------------------------------------------------------

void FUltraWireSettingsCustomization::RefreshSavedPresetList()
{
	SavedPresetNames.Reset();

	const UUltraWireSettings* Settings = UUltraWireSettings::Get();
	if (!Settings)
	{
		return;
	}

	for (const FUltraWirePreset& Preset : Settings->SavedPresets)
	{
		SavedPresetNames.Add(MakeShareable(new FString(Preset.PresetName)));
	}

	if (SavedPresetNames.Num() > 0 && !SelectedSavedPreset.IsValid())
	{
		SelectedSavedPreset = SavedPresetNames[0];
	}
}

TSharedRef<SWidget> FUltraWireSettingsCustomization::MakeSavedPresetComboEntry(
	TSharedPtr<FString> Item)
{
	return SNew(STextBlock)
		.Text(Item.IsValid() ? FText::FromString(*Item) : FText::GetEmpty())
		.Margin(FMargin(4.0f, 2.0f));
}

FText FUltraWireSettingsCustomization::GetSavedPresetComboText() const
{
	if (SelectedSavedPreset.IsValid() && !SelectedSavedPreset->IsEmpty())
	{
		return FText::FromString(*SelectedSavedPreset);
	}
	return FText::FromString(TEXT("(No Presets)"));
}

void FUltraWireSettingsCustomization::OnSavedPresetSelected(
	TSharedPtr<FString> Item,
	ESelectInfo::Type   SelectInfo)
{
	SelectedSavedPreset = Item;
}

// ---------------------------------------------------------------------------
// Built-in preset combo helpers
// ---------------------------------------------------------------------------

TSharedRef<SWidget> FUltraWireSettingsCustomization::MakeBuiltinPresetComboEntry(
	TSharedPtr<FString> Item)
{
	return SNew(STextBlock)
		.Text(Item.IsValid() ? FText::FromString(*Item) : FText::GetEmpty())
		.Margin(FMargin(4.0f, 2.0f));
}

FText FUltraWireSettingsCustomization::GetBuiltinPresetComboText() const
{
	if (SelectedBuiltinPreset.IsValid() && !SelectedBuiltinPreset->IsEmpty())
	{
		return FText::FromString(*SelectedBuiltinPreset);
	}
	return FText::FromString(TEXT("(Select Preset)"));
}

void FUltraWireSettingsCustomization::OnBuiltinPresetSelected(
	TSharedPtr<FString> Item,
	ESelectInfo::Type   SelectInfo)
{
	SelectedBuiltinPreset = Item;
}

// ---------------------------------------------------------------------------
// Graph profile combo helpers
// ---------------------------------------------------------------------------

TSharedRef<SWidget> FUltraWireSettingsCustomization::MakeGraphPresetComboEntry(
	TSharedPtr<FString> Item)
{
	return SNew(STextBlock)
		.Text(Item.IsValid() ? FText::FromString(*Item) : FText::GetEmpty())
		.Margin(FMargin(4.0f, 2.0f));
}

void FUltraWireSettingsCustomization::OnGraphPresetSelected(
	TSharedPtr<FString> Item,
	ESelectInfo::Type   SelectInfo,
	EUltraWireGraphType GraphType)
{
	SelectedGraphPresets.FindOrAdd(GraphType) = Item;

	if (!Item.IsValid() || Item->IsEmpty() || *Item == TEXT("(Default)"))
	{
		// Remove override so the default profile is used.
		UUltraWireSettings* Settings = UUltraWireSettings::Get();
		if (Settings)
		{
			Settings->GraphProfileOverrides.Remove(GraphType);
			Settings->SaveConfig();
		}
		return;
	}

	// Load the selected preset and store it as the override for this graph type.
	UUltraWireSettings* Settings = UUltraWireSettings::Get();
	if (Settings)
	{
		FUltraWireProfile Profile;
		if (Settings->LoadPreset(*Item, Profile))
		{
			Settings->GraphProfileOverrides.FindOrAdd(GraphType) = Profile;
			Settings->SaveConfig();
			Settings->OnSettingsChanged.Broadcast();
		}
	}
}

FText FUltraWireSettingsCustomization::GetGraphPresetComboText(EUltraWireGraphType GraphType) const
{
	const TSharedPtr<FString>* Found = SelectedGraphPresets.Find(GraphType);
	if (Found && Found->IsValid() && !(*Found)->IsEmpty())
	{
		return FText::FromString(**Found);
	}
	return FText::FromString(TEXT("(Default)"));
}

// ---------------------------------------------------------------------------
// Button handlers
// ---------------------------------------------------------------------------

FReply FUltraWireSettingsCustomization::OnImportPresetClicked()
{
	IDesktopPlatform* DesktopPlatform = FDesktopPlatformModule::Get();
	if (!DesktopPlatform)
	{
		return FReply::Handled();
	}

	TArray<FString> SelectedFiles;
	const FString DefaultPath = FPaths::ProjectSavedDir();
	const bool bOpened = DesktopPlatform->OpenFileDialog(
		FSlateApplication::Get().FindBestParentWindowHandleForDialogs(nullptr),
		TEXT("Import UltraWire Preset"),
		DefaultPath,
		TEXT(""),
		TEXT("UltraWire Preset (*.json)|*.json|All Files (*.*)|*.*"),
		EFileDialogFlags::None,
		SelectedFiles);

	if (bOpened && SelectedFiles.Num() > 0)
	{
		UUltraWireSettings* Settings = UUltraWireSettings::Get();
		if (Settings)
		{
			const bool bSuccess = Settings->ImportPresetFromFile(SelectedFiles[0]);
			if (bSuccess)
			{
				RefreshSavedPresetList();

				// Force the detail panel to rebuild so the new preset appears
				// in all combo boxes.
				if (CachedDetailBuilderPtr)
				{
					CachedDetailBuilderPtr->ForceRefreshDetails();
				}

				UE_LOG(LogUltraWireSettingsUI, Log,
				       TEXT("Imported preset from: %s"), *SelectedFiles[0]);
			}
			else
			{
				UE_LOG(LogUltraWireSettingsUI, Warning,
				       TEXT("Failed to import preset from: %s"), *SelectedFiles[0]);
			}
		}
	}

	return FReply::Handled();
}

FReply FUltraWireSettingsCustomization::OnExportPresetClicked()
{
	if (!SelectedSavedPreset.IsValid() || SelectedSavedPreset->IsEmpty())
	{
		UE_LOG(LogUltraWireSettingsUI, Warning,
		       TEXT("Cannot export – no preset is selected."));
		return FReply::Handled();
	}

	IDesktopPlatform* DesktopPlatform = FDesktopPlatformModule::Get();
	if (!DesktopPlatform)
	{
		return FReply::Handled();
	}

	TArray<FString> SelectedFiles;
	const FString DefaultPath = FPaths::ProjectSavedDir();
	const FString DefaultName = *SelectedSavedPreset + TEXT(".json");

	const bool bSaved = DesktopPlatform->SaveFileDialog(
		FSlateApplication::Get().FindBestParentWindowHandleForDialogs(nullptr),
		TEXT("Export UltraWire Preset"),
		DefaultPath,
		DefaultName,
		TEXT("UltraWire Preset (*.json)|*.json|All Files (*.*)|*.*"),
		EFileDialogFlags::None,
		SelectedFiles);

	if (bSaved && SelectedFiles.Num() > 0)
	{
		const UUltraWireSettings* Settings = UUltraWireSettings::Get();
		if (Settings)
		{
			const bool bSuccess =
				Settings->ExportPresetToFile(*SelectedSavedPreset, SelectedFiles[0]);

			if (bSuccess)
			{
				UE_LOG(LogUltraWireSettingsUI, Log,
				       TEXT("Exported preset '%s' to: %s"),
				       **SelectedSavedPreset, *SelectedFiles[0]);
			}
			else
			{
				UE_LOG(LogUltraWireSettingsUI, Warning,
				       TEXT("Failed to export preset '%s'."), **SelectedSavedPreset);
			}
		}
	}

	return FReply::Handled();
}

FReply FUltraWireSettingsCustomization::OnApplyBuiltinPresetClicked()
{
	if (!SelectedBuiltinPreset.IsValid() || SelectedBuiltinPreset->IsEmpty())
	{
		UE_LOG(LogUltraWireSettingsUI, Warning,
		       TEXT("No built-in preset selected."));
		return FReply::Handled();
	}

	FUltraWireProfile Profile;
	if (!GetBuiltinPresetProfile(*SelectedBuiltinPreset, Profile))
	{
		UE_LOG(LogUltraWireSettingsUI, Warning,
		       TEXT("Unknown built-in preset: %s"), **SelectedBuiltinPreset);
		return FReply::Handled();
	}

	UUltraWireSettings* Settings = UUltraWireSettings::Get();
	if (Settings)
	{
		Settings->DefaultProfile = Profile;
		Settings->SaveConfig();
		Settings->OnSettingsChanged.Broadcast();

		UE_LOG(LogUltraWireSettingsUI, Log,
		       TEXT("Applied built-in preset '%s' to Default Profile."),
		       **SelectedBuiltinPreset);

		// Refresh so the Default Profile category displays the new values.
		if (CachedDetailBuilderPtr)
		{
			CachedDetailBuilderPtr->ForceRefreshDetails();
		}
	}

	return FReply::Handled();
}

// ---------------------------------------------------------------------------
// Built-in preset definitions
// ---------------------------------------------------------------------------

TArray<FUltraWireSettingsCustomization::FBuiltinPreset>
FUltraWireSettingsCustomization::GetBuiltinPresets()
{
	return {
		{ UltraWireBuiltinPresets::Clean,     TEXT("Thin, sharp, no decorations.") },
		{ UltraWireBuiltinPresets::Neon,      TEXT("Bright glow, dark theme, vibrant colors.") },
		{ UltraWireBuiltinPresets::Blueprint, TEXT("Mimics the standard Blueprint editor look.") },
		{ UltraWireBuiltinPresets::Circuit,   TEXT("Manhattan routing, no frills.") },
		{ UltraWireBuiltinPresets::Retro,     TEXT("Rounded corners, muted palette, subtle glow.") },
	};
}

bool FUltraWireSettingsCustomization::GetBuiltinPresetProfile(
	const FString&     PresetName,
	FUltraWireProfile& OutProfile)
{
	OutProfile = FUltraWireProfile(); // Reset to defaults first.

	// -----------------------------------------------------------------------
	// 1. Clean & Minimal
	// -----------------------------------------------------------------------
	if (PresetName == UltraWireBuiltinPresets::Clean)
	{
		OutProfile.WireStyle         = EUltraWireStyle::Manhattan;
		OutProfile.CornerStyle       = EUltraWireCornerStyle::Sharp;
		OutProfile.CornerRadius      = 0.0f;
		OutProfile.WireThickness     = 1.0f;
		OutProfile.bEnableRibbons    = false;
		OutProfile.CrossingStyle     = EUltraWireCrossingStyle::None;
		OutProfile.bEnableBubbles    = false;
		OutProfile.bEnableGlow       = false;
		OutProfile.bEnableSmartRouting = false;
		OutProfile.bEnableHeatmap    = false;
		OutProfile.bEnableNodeTheming= false;
		OutProfile.bEnableAutoLabels = false;
		OutProfile.bEnableMinimap    = false;
		return true;
	}

	// -----------------------------------------------------------------------
	// 2. Neon Glow
	// -----------------------------------------------------------------------
	if (PresetName == UltraWireBuiltinPresets::Neon)
	{
		OutProfile.WireStyle         = EUltraWireStyle::Subway;
		OutProfile.CornerStyle       = EUltraWireCornerStyle::Rounded;
		OutProfile.CornerRadius      = 12.0f;
		OutProfile.WireThickness     = 2.5f;
		OutProfile.bEnableRibbons    = false;
		OutProfile.CrossingStyle     = EUltraWireCrossingStyle::Arc;
		OutProfile.CrossingSize      = 10.0f;
		OutProfile.bEnableBubbles    = true;
		OutProfile.BubbleSpeed       = 180.0f;
		OutProfile.BubbleSize        = 4.0f;
		OutProfile.BubbleSpacing     = 60.0f;
		OutProfile.bEnableGlow       = true;
		OutProfile.GlowIntensity     = 2.5f;
		OutProfile.GlowWidth         = 10.0f;
		OutProfile.bGlowPulse        = true;
		OutProfile.GlowPulseSpeed    = 1.5f;
		OutProfile.bEnableSmartRouting = true;
		OutProfile.RoutingGridSize   = 20;
		OutProfile.RoutingNodePadding= 12.0f;
		OutProfile.bEnableHeatmap    = false;
		OutProfile.bEnableNodeTheming= true;
		OutProfile.NodeBodyOpacity   = 0.85f;
		OutProfile.NodeHeaderTintColor = FLinearColor(0.05f, 0.0f, 0.15f, 1.0f);
		OutProfile.NodeCornerRadius  = 6.0f;
		OutProfile.PinShape          = EUltraWirePinShape::Circle;
		OutProfile.bEnableAutoLabels = false;
		OutProfile.bEnableMinimap    = true;
		OutProfile.MinimapPosition   = EUltraWireMinimapPosition::BottomRight;
		OutProfile.MinimapSize       = 220.0f;
		OutProfile.MinimapOpacity    = 0.75f;
		return true;
	}

	// -----------------------------------------------------------------------
	// 3. Blueprint Classic
	// -----------------------------------------------------------------------
	if (PresetName == UltraWireBuiltinPresets::Blueprint)
	{
		OutProfile.WireStyle         = EUltraWireStyle::Default;
		OutProfile.CornerStyle       = EUltraWireCornerStyle::Rounded;
		OutProfile.CornerRadius      = 8.0f;
		OutProfile.WireThickness     = 1.5f;
		OutProfile.bEnableRibbons    = false;
		OutProfile.CrossingStyle     = EUltraWireCrossingStyle::None;
		OutProfile.bEnableBubbles    = true;
		OutProfile.BubbleSpeed       = 100.0f;
		OutProfile.BubbleSize        = 3.0f;
		OutProfile.BubbleSpacing     = 50.0f;
		OutProfile.bEnableGlow       = false;
		OutProfile.bEnableSmartRouting = false;
		OutProfile.bEnableHeatmap    = false;
		OutProfile.bEnableNodeTheming= false;
		OutProfile.bEnableAutoLabels = false;
		OutProfile.bEnableMinimap    = false;
		return true;
	}

	// -----------------------------------------------------------------------
	// 4. Circuit Board
	// -----------------------------------------------------------------------
	if (PresetName == UltraWireBuiltinPresets::Circuit)
	{
		OutProfile.WireStyle         = EUltraWireStyle::Manhattan;
		OutProfile.CornerStyle       = EUltraWireCornerStyle::Sharp;
		OutProfile.CornerRadius      = 0.0f;
		OutProfile.WireThickness     = 1.5f;
		OutProfile.bEnableRibbons    = true;
		OutProfile.RibbonOffset      = 5.0f;
		OutProfile.CrossingStyle     = EUltraWireCrossingStyle::Gap;
		OutProfile.CrossingSize      = 6.0f;
		OutProfile.bEnableBubbles    = false;
		OutProfile.bEnableGlow       = false;
		OutProfile.bEnableSmartRouting = true;
		OutProfile.RoutingGridSize   = 15;
		OutProfile.RoutingNodePadding= 8.0f;
		OutProfile.bEnableHeatmap    = true;
		OutProfile.HeatmapColdColor  = FLinearColor(0.0f, 0.6f, 0.2f, 1.0f);
		OutProfile.HeatmapHotColor   = FLinearColor(1.0f, 0.3f, 0.0f, 1.0f);
		OutProfile.bEnableNodeTheming= true;
		OutProfile.NodeBodyOpacity   = 1.0f;
		OutProfile.NodeHeaderTintColor = FLinearColor(0.02f, 0.08f, 0.02f, 1.0f);
		OutProfile.NodeCornerRadius  = 2.0f;
		OutProfile.PinShape          = EUltraWirePinShape::Square;
		OutProfile.bEnableAutoLabels = true;
		OutProfile.bEnableMinimap    = true;
		OutProfile.MinimapPosition   = EUltraWireMinimapPosition::TopRight;
		OutProfile.MinimapSize       = 180.0f;
		OutProfile.MinimapOpacity    = 0.9f;
		return true;
	}

	// -----------------------------------------------------------------------
	// 5. Retro Terminal
	// -----------------------------------------------------------------------
	if (PresetName == UltraWireBuiltinPresets::Retro)
	{
		OutProfile.WireStyle         = EUltraWireStyle::Freeform;
		OutProfile.FreeformAngle     = 45.0f;
		OutProfile.CornerStyle       = EUltraWireCornerStyle::Chamfered;
		OutProfile.CornerRadius      = 10.0f;
		OutProfile.WireThickness     = 2.0f;
		OutProfile.bEnableRibbons    = false;
		OutProfile.CrossingStyle     = EUltraWireCrossingStyle::Circle;
		OutProfile.CrossingSize      = 7.0f;
		OutProfile.bEnableBubbles    = true;
		OutProfile.BubbleSpeed       = 60.0f;
		OutProfile.BubbleSize        = 2.5f;
		OutProfile.BubbleSpacing     = 80.0f;
		OutProfile.bEnableGlow       = true;
		OutProfile.GlowIntensity     = 1.2f;
		OutProfile.GlowWidth         = 5.0f;
		OutProfile.bGlowPulse        = false;
		OutProfile.bEnableSmartRouting = false;
		OutProfile.bEnableHeatmap    = false;
		OutProfile.bEnableNodeTheming= true;
		OutProfile.NodeBodyOpacity   = 0.9f;
		OutProfile.NodeHeaderTintColor = FLinearColor(0.05f, 0.15f, 0.05f, 1.0f);
		OutProfile.NodeCornerRadius  = 0.0f;
		OutProfile.PinShape          = EUltraWirePinShape::Diamond;
		OutProfile.bEnableAutoLabels = false;
		OutProfile.bEnableMinimap    = true;
		OutProfile.MinimapPosition   = EUltraWireMinimapPosition::BottomLeft;
		OutProfile.MinimapSize       = 200.0f;
		OutProfile.MinimapOpacity    = 0.7f;
		return true;
	}

	return false;
}
