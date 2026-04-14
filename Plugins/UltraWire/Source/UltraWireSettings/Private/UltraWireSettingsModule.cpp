// Copyright 2026 StraySpark. All Rights Reserved.

#include "UltraWireSettingsModule.h"
#include "UltraWireSettingsCustomization.h"
#include "UltraWireSettings.h"

#include "PropertyEditorModule.h"
#include "Modules/ModuleManager.h"
#include "Logging/LogMacros.h"

DEFINE_LOG_CATEGORY_STATIC(LogUltraWireSettings, Log, All);

// ---------------------------------------------------------------------------
// IModuleInterface
// ---------------------------------------------------------------------------

void FUltraWireSettingsModule::StartupModule()
{
	UE_LOG(LogUltraWireSettings, Log, TEXT("UltraWireSettings: StartupModule"));

	// Retrieve the PropertyEditor module so we can register our customization.
	// We use GetModuleChecked here: if PropertyEditor isn't loaded yet that is
	// a configuration error and we want to surface it loudly.
	FPropertyEditorModule& PropertyEditorModule =
		FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");

	// Register the detail customization for UUltraWireSettings.  The class name
	// must match the C++ class name exactly (without the 'U' prefix that UHT
	// strips).
	PropertyEditorModule.RegisterCustomClassLayout(
		UUltraWireSettings::StaticClass()->GetFName(),
		FOnGetDetailCustomizationInstance::CreateStatic(
			&FUltraWireSettingsCustomization::MakeInstance));

	// Notify the property editor that the layout for this class has changed so
	// any already-open detail panels refresh themselves.
	PropertyEditorModule.NotifyCustomizationModuleChanged();

	UE_LOG(LogUltraWireSettings, Log,
	       TEXT("UltraWireSettings: Registered detail customization for UUltraWireSettings."));
}

void FUltraWireSettingsModule::ShutdownModule()
{
	UE_LOG(LogUltraWireSettings, Log, TEXT("UltraWireSettings: ShutdownModule"));

	// Unregister the customization.  PropertyEditor may have already been
	// unloaded if the editor is shutting down, so check before accessing it.
	if (FModuleManager::Get().IsModuleLoaded("PropertyEditor"))
	{
		FPropertyEditorModule& PropertyEditorModule =
			FModuleManager::GetModuleChecked<FPropertyEditorModule>("PropertyEditor");

		PropertyEditorModule.UnregisterCustomClassLayout(
			UUltraWireSettings::StaticClass()->GetFName());

		PropertyEditorModule.NotifyCustomizationModuleChanged();
	}

	UE_LOG(LogUltraWireSettings, Log,
	       TEXT("UltraWireSettings: Detail customization unregistered."));
}

// ---------------------------------------------------------------------------
// Module registration
// ---------------------------------------------------------------------------

IMPLEMENT_MODULE(FUltraWireSettingsModule, UltraWireSettings)
