// Copyright 2026 StraySpark. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleInterface.h"
#include "Modules/ModuleManager.h"

/**
 * UltraWireSettings Module
 *
 * Editor-only module that:
 *   1. Registers FUltraWireSettingsCustomization as the IDetailCustomization
 *      class for UUltraWireSettings objects displayed in the Project Settings
 *      and Plugin Settings panels.
 *   2. Ensures the customization is unregistered cleanly on module shutdown
 *      to avoid dangling pointers inside the PropertyEditor subsystem.
 *
 * The module has no public API beyond the standard IModuleInterface methods
 * and the static accessor below.  All user-visible behaviour is implemented
 * in FUltraWireSettingsCustomization.
 *
 * This module must only be loaded in editor builds.  Use LoadModuleChecked
 * rather than IsModuleLoaded where possible so misconfigured .uproject files
 * produce clear error messages.
 */
class ULTRAWIRESETTINGS_API FUltraWireSettingsModule : public IModuleInterface
{
public:
	/** IModuleInterface overrides */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

	/**
	 * Convenience accessor.
	 * Safe to call from any editor thread after the module has been loaded.
	 */
	static FUltraWireSettingsModule& Get()
	{
		return FModuleManager::LoadModuleChecked<FUltraWireSettingsModule>("UltraWireSettings");
	}

	/** Returns true if the module is currently loaded. */
	static bool IsAvailable()
	{
		return FModuleManager::Get().IsModuleLoaded("UltraWireSettings");
	}
};
