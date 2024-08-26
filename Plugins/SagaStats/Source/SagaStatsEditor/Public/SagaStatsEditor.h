#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

struct FGraphPanelPinFactory;

class FSagaStatsEditorModule : public IModuleInterface
{
public:
    virtual void StartupModule() override;
    virtual void ShutdownModule() override;

    static FSagaStatsEditorModule& Get()
    {
        static const FName ModuleName = "SagaStatsEditor";
        return FModuleManager::LoadModuleChecked<FSagaStatsEditorModule>(ModuleName);
    }

    void PreloadAssetsByClass(UClass* InClass) const;

    void OnPostEngineInit();

private:
    /** Pin factory for gameplay abilities; Cached so it can be unregistered */
    TSharedPtr<FGraphPanelPinFactory> GameplayAbilitiesGraphPanelPinFactory;
};
