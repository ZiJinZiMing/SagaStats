// Copyright 2026 StraySpark. All Rights Reserved.

#include "UltraWireSettings.h"
#include "Misc/FileHelper.h"
#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonWriter.h"
#include "JsonObjectConverter.h"

UUltraWireSettings::UUltraWireSettings()
{
}

UUltraWireSettings* UUltraWireSettings::Get()
{
	return GetMutableDefault<UUltraWireSettings>();
}

const FUltraWireProfile& UUltraWireSettings::GetProfileForGraphType(EUltraWireGraphType GraphType) const
{
	if (const FUltraWireProfile* Override = GraphProfileOverrides.Find(GraphType))
	{
		return *Override;
	}
	return DefaultProfile;
}

void UUltraWireSettings::SavePreset(const FString& Name, const FString& Description, const FUltraWireProfile& Profile)
{
	// Remove existing preset with same name
	SavedPresets.RemoveAll([&Name](const FUltraWirePreset& Preset)
	{
		return Preset.PresetName == Name;
	});

	FUltraWirePreset NewPreset;
	NewPreset.PresetName = Name;
	NewPreset.Description = Description;
	NewPreset.Profile = Profile;
	SavedPresets.Add(NewPreset);

	SaveConfig();
	OnSettingsChanged.Broadcast();
}

bool UUltraWireSettings::LoadPreset(const FString& Name, FUltraWireProfile& OutProfile) const
{
	for (const FUltraWirePreset& Preset : SavedPresets)
	{
		if (Preset.PresetName == Name)
		{
			OutProfile = Preset.Profile;
			return true;
		}
	}
	return false;
}

bool UUltraWireSettings::DeletePreset(const FString& Name)
{
	int32 Removed = SavedPresets.RemoveAll([&Name](const FUltraWirePreset& Preset)
	{
		return Preset.PresetName == Name;
	});

	if (Removed > 0)
	{
		SaveConfig();
		OnSettingsChanged.Broadcast();
		return true;
	}
	return false;
}

bool UUltraWireSettings::ExportPresetToFile(const FString& Name, const FString& FilePath) const
{
	FUltraWireProfile Profile;
	if (!LoadPreset(Name, Profile))
	{
		return false;
	}

	FUltraWirePreset ExportPreset;
	ExportPreset.PresetName = Name;
	ExportPreset.Profile = Profile;

	FString JsonString;
	FJsonObjectConverter::UStructToJsonObjectString(ExportPreset, JsonString);

	return FFileHelper::SaveStringToFile(JsonString, *FilePath);
}

bool UUltraWireSettings::ImportPresetFromFile(const FString& FilePath)
{
	FString JsonString;
	if (!FFileHelper::LoadFileToString(JsonString, *FilePath))
	{
		return false;
	}

	FUltraWirePreset ImportedPreset;
	if (!FJsonObjectConverter::JsonObjectStringToUStruct(JsonString, &ImportedPreset))
	{
		return false;
	}

	if (ImportedPreset.PresetName.IsEmpty())
	{
		ImportedPreset.PresetName = FPaths::GetBaseFilename(FilePath);
	}

	SavePreset(ImportedPreset.PresetName, ImportedPreset.Description, ImportedPreset.Profile);
	return true;
}

#if WITH_EDITOR
void UUltraWireSettings::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	OnSettingsChanged.Broadcast();
}
#endif
