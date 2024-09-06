/******************************************************************************************
* Plugin:       SagaStats
* Author:       Jinming Zhang
* Description:  SagaStats is a status system that supports fully blueprintable attribute definitions and value calculations.
******************************************************************************************/

#include "Utils/SSEditorUtils.h"

#include "AttributeSet.h"
#include "DetailLayoutBuilder.h"
#include "Editor.h"
#include "Editor/SSBlueprintEditor.h"
#include "Misc/EngineVersionComparison.h"
#include "Subsystems/AssetEditorSubsystem.h"

UAttributeSet* UE::SS::EditorUtils::GetAttributeBeingCustomized(const IDetailLayoutBuilder& InDetailLayout)
{
	TArray<TWeakObjectPtr<UObject>> ObjectsBeingCustomized;
	InDetailLayout.GetObjectsBeingCustomized(ObjectsBeingCustomized);

	TArray<UAttributeSet*> AttributeSetsBeingCustomized;
	for (TWeakObjectPtr<UObject> ObjectBeingCustomized : ObjectsBeingCustomized)
	{
		if (UAttributeSet* AttributeSet = Cast<UAttributeSet>(ObjectBeingCustomized.Get()))
		{
			AttributeSetsBeingCustomized.Add(AttributeSet);
		}
	}

	return AttributeSetsBeingCustomized.IsValidIndex(0) ? AttributeSetsBeingCustomized[0] : nullptr;
}

UBlueprint* UE::SS::EditorUtils::GetBlueprintFromClass(const UClass* InClass)
{
	if (!InClass)
	{
		return nullptr;
	}

	return Cast<UBlueprint>(InClass->ClassGeneratedBy);
}

TWeakPtr<FSSBlueprintEditor> UE::SS::EditorUtils::FindBlueprintEditorForAsset(UObject* InObject)
{
	if (!GEditor || !IsValid(InObject))
	{
		return nullptr;
	}

	UAssetEditorSubsystem* AssetEditorSubsystem = GEditor->GetEditorSubsystem<UAssetEditorSubsystem>();
	if (!IsValid(AssetEditorSubsystem))
	{
		return nullptr;
	}

	IAssetEditorInstance* EditorInstance = AssetEditorSubsystem->FindEditorForAsset(InObject, false);
	if (!EditorInstance)
	{
		return nullptr;
	}

	FSSBlueprintEditor* BlueprintEditor = StaticCast<FSSBlueprintEditor*>(EditorInstance);
	if (!BlueprintEditor)
	{
		return nullptr;
	}
#if UE_VERSION_NEWER_THAN(5, 1, -1)
	return StaticCastWeakPtr<FSSBlueprintEditor>(BlueprintEditor->AsWeak());
#else
	TWeakPtr<FSSBlueprintEditor> WeakPtr(StaticCastSharedRef<FSSBlueprintEditor>(BlueprintEditor->AsShared()));
	return WeakPtr;
#endif
}
