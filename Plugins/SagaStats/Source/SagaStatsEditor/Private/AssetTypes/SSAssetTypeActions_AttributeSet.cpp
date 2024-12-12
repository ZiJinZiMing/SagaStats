/******************************************************************************************
* Plugin:       SagaStats
* Author:       Jinming Zhang
* Description:  SagaStats is a status system that supports fully blueprintable attribute definitions and value calculations.
******************************************************************************************/
#pragma once

#include "AssetTypes/SSAssetTypeActions_AttributeSet.h"

#include "SagaAttributeSet.h"
#include "AssetTypes/SSBlueprintFactory.h"
#include "Editor/SSBlueprintEditor.h"
#include "Misc/MessageDialog.h"

#define LOCTEXT_NAMESPACE "SSAssetTypeActions_AttributeSet"

FSSAssetTypeActions_AttributeSet::FSSAssetTypeActions_AttributeSet(const EAssetTypeCategories::Type InAssetCategory)
	: AssetCategory(InAssetCategory)
{
}

void FSSAssetTypeActions_AttributeSet::OpenAssetEditor(const TArray<UObject*>& InObjects, const TSharedPtr<IToolkitHost> EditWithinLevelEditor)
{
	const EToolkitMode::Type Mode = EditWithinLevelEditor.IsValid() ? EToolkitMode::WorldCentric : EToolkitMode::Standalone;

	for (auto ObjIt = InObjects.CreateConstIterator(); ObjIt; ++ObjIt)
	{
		if (UBlueprint* Blueprint = Cast<UBlueprint>(*ObjIt))
		{
			const TSharedRef<FSSBlueprintEditor> NewEditor(new FSSBlueprintEditor());

			TArray<UBlueprint*> Blueprints;
			Blueprints.Add(Blueprint);

			NewEditor->InitAttributeSetEditor(Mode, EditWithinLevelEditor, Blueprints, false);
		}
		else
		{
			FMessageDialog::Open(EAppMsgType::Ok, LOCTEXT("FailedToLoadAttributeSetBlueprint", "Attribute Set Blueprint could not be loaded because it derives from an invalid class. Check to make sure the parent class for this blueprint hasn't been removed!"));
		}
	}
}

const TArray<FText>& FSSAssetTypeActions_AttributeSet::GetSubMenus() const
{
	static const TArray SubMenus
	{
		LOCTEXT("AssetTypeActions_BlueprintAttributesSubMenu", "SagaStats")
	};

	return SubMenus;
}

UFactory* FSSAssetTypeActions_AttributeSet::GetFactoryForBlueprintType(UBlueprint* InBlueprint) const
{
	check(InBlueprint && InBlueprint->IsA(USagaAttributeSetBlueprint::StaticClass()));
	USSBlueprintFactory* Factory = NewObject<USSBlueprintFactory>();
	Factory->ParentClass = TSubclassOf<USagaAttributeSet>(*InBlueprint->GeneratedClass);
	return Factory;
}

#undef LOCTEXT_NAMESPACE
