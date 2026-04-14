// Copyright 2026 StraySpark. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

class FUltraWireThemeEngine;

/**
 * UltraWireTheme module.
 *
 * Responsible for:
 *   - Constructing and owning the FUltraWireThemeEngine singleton.
 *   - Subscribing to UUltraWireSettings::OnSettingsChanged so that any
 *     property edit in the Project Settings panel is immediately reflected
 *     in the Slate style overrides without restarting the editor.
 *   - Applying the initial theme on startup when the plugin is enabled.
 *   - Tearing everything down cleanly on shutdown so that no dangling
 *     FSlateStyleSet pointers are left behind.
 */
class ULTRAWIRETHEME_API FUltraWireThemeModule : public IModuleInterface
{
public:

	// IModuleInterface
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

	/** Convenience accessor – valid between StartupModule and ShutdownModule. */
	static FUltraWireThemeModule& Get();
	static bool IsAvailable();

	/** Returns the theme engine owned by this module. Never null while the module is loaded. */
	FUltraWireThemeEngine* GetThemeEngine() const { return ThemeEngine.Get(); }

private:

	/** Called whenever UUltraWireSettings::OnSettingsChanged broadcasts. */
	void OnSettingsChanged();

	/** Handle stored so we can unregister on shutdown. */
	FDelegateHandle SettingsChangedHandle;

	/** Owned theme engine instance. */
	TUniquePtr<FUltraWireThemeEngine> ThemeEngine;
};
