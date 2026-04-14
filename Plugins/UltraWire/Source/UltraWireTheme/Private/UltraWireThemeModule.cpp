// Copyright 2026 StraySpark. All Rights Reserved.

#include "UltraWireThemeModule.h"
#include "UltraWireThemeEngine.h"
#include "UltraWireSettings.h"
#include "UltraWireTypes.h"
#include "Modules/ModuleManager.h"

#define LOCTEXT_NAMESPACE "FUltraWireThemeModule"

// ---------------------------------------------------------------------------
// IModuleInterface
// ---------------------------------------------------------------------------

void FUltraWireThemeModule::StartupModule()
{
	// Construct the engine early so that OnSettingsChanged can always
	// delegate to a valid object.
	ThemeEngine = MakeUnique<FUltraWireThemeEngine>();

	// Subscribe to hot-reload notifications from the settings object.
	// UDeveloperSettings subclasses are CDO singletons, so Get() is always
	// valid in editor builds where this module loads.
	if (UUltraWireSettings* Settings = UUltraWireSettings::Get())
	{
		SettingsChangedHandle = Settings->OnSettingsChanged.AddRaw(
			this, &FUltraWireThemeModule::OnSettingsChanged);

		// Perform the initial apply if the plugin is enabled.
		if (Settings->bEnabled && Settings->DefaultProfile.bEnableNodeTheming)
		{
			ThemeEngine->ApplyTheme(Settings->DefaultProfile);
		}
	}
}

void FUltraWireThemeModule::ShutdownModule()
{
	// Unsubscribe before the settings object could be destroyed.
	if (UUltraWireSettings* Settings = UUltraWireSettings::Get())
	{
		Settings->OnSettingsChanged.Remove(SettingsChangedHandle);
	}
	SettingsChangedHandle.Reset();

	// Reset restores the original Slate styles before the engine is destroyed.
	if (ThemeEngine.IsValid())
	{
		ThemeEngine->ResetTheme();
		ThemeEngine.Reset();
	}
}

// ---------------------------------------------------------------------------
// Static accessors
// ---------------------------------------------------------------------------

FUltraWireThemeModule& FUltraWireThemeModule::Get()
{
	return FModuleManager::LoadModuleChecked<FUltraWireThemeModule>("UltraWireTheme");
}

bool FUltraWireThemeModule::IsAvailable()
{
	return FModuleManager::Get().IsModuleLoaded("UltraWireTheme");
}

// ---------------------------------------------------------------------------
// Settings hot-reload
// ---------------------------------------------------------------------------

void FUltraWireThemeModule::OnSettingsChanged()
{
	if (!ThemeEngine.IsValid())
	{
		return;
	}

	const UUltraWireSettings* Settings = UUltraWireSettings::Get();
	if (!Settings)
	{
		return;
	}

	if (Settings->bEnabled && Settings->DefaultProfile.bEnableNodeTheming)
	{
		ThemeEngine->ApplyTheme(Settings->DefaultProfile);
	}
	else
	{
		// Plugin disabled or node theming turned off – restore vanilla look.
		ThemeEngine->ResetTheme();
	}
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FUltraWireThemeModule, UltraWireTheme)
