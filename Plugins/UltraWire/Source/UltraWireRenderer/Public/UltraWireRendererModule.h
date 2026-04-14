// Copyright 2026 StraySpark. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleInterface.h"
#include "Modules/ModuleManager.h"

class FUltraWirePinConnectionFactory;

/**
 * UltraWireRenderer module.
 *
 * Owns the lifetime of all FGraphPanelPinConnectionFactory instances registered
 * with FEdGraphUtilities. On startup the module creates one factory per supported
 * graph type and registers them; on shutdown it unregisters and destroys them.
 */
class ULTRAWIRERENDERER_API FUltraWireRendererModule : public IModuleInterface
{
public:
	/** IModuleInterface overrides */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

	/** Convenience accessor – safe to call from any editor thread. */
	static FUltraWireRendererModule& Get()
	{
		return FModuleManager::LoadModuleChecked<FUltraWireRendererModule>("UltraWireRenderer");
	}

	static bool IsAvailable()
	{
		return FModuleManager::Get().IsModuleLoaded("UltraWireRenderer");
	}

private:
	/** All registered factories, kept alive for the duration of the module. */
	TArray<TSharedPtr<FUltraWirePinConnectionFactory>> RegisteredFactories;
};
