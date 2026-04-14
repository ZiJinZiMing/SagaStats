// Copyright 2026 StraySpark. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "UltraWireTypes.h"
#include "UltraWireSettings.generated.h"

UCLASS(config = EditorPerProjectUserSettings, defaultconfig, meta = (DisplayName = "UltraWire"))
class ULTRAWIRECORE_API UUltraWireSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	UUltraWireSettings();

	static UUltraWireSettings* Get();

	// Master enable
	UPROPERTY(config, EditAnywhere, Category = "General")
	bool bEnabled = true;

	// Default profile applied to all graph types unless overridden
	UPROPERTY(config, EditAnywhere, Category = "Default Profile")
	FUltraWireProfile DefaultProfile;

	// Per-graph-type profile overrides
	UPROPERTY(config, EditAnywhere, Category = "Graph Profiles")
	TMap<EUltraWireGraphType, FUltraWireProfile> GraphProfileOverrides;

	// Saved presets
	UPROPERTY(config, EditAnywhere, Category = "Presets")
	TArray<FUltraWirePreset> SavedPresets;

	// Get the effective profile for a given graph type
	const FUltraWireProfile& GetProfileForGraphType(EUltraWireGraphType GraphType) const;

	// Preset management
	void SavePreset(const FString& Name, const FString& Description, const FUltraWireProfile& Profile);
	bool LoadPreset(const FString& Name, FUltraWireProfile& OutProfile) const;
	bool DeletePreset(const FString& Name);
	bool ExportPresetToFile(const FString& Name, const FString& FilePath) const;
	bool ImportPresetFromFile(const FString& FilePath);

	// Hot reload support
	DECLARE_MULTICAST_DELEGATE(FOnSettingsChanged);
	FOnSettingsChanged OnSettingsChanged;

	virtual FName GetCategoryName() const override { return FName(TEXT("Plugins")); }
	virtual FName GetSectionName() const override { return FName(TEXT("UltraWire")); }

#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
};
