// DamageOperationBlueprintFactory.cpp
#include "AssetTypes/DamageOperationBlueprintFactory.h"
#include "SagaStatsEditor.h"
#include "DamageFramework/DamageOperationBase.h"
#include "Kismet2/KismetEditorUtilities.h"
#include "Engine/Blueprint.h"
#include "Engine/BlueprintGeneratedClass.h"

#define LOCTEXT_NAMESPACE "DamageOperationBlueprintFactory"

UDamageOperationBlueprintFactory::UDamageOperationBlueprintFactory()
{
	bCreateNew = true;
	bEditAfterNew = true;
	SupportedClass = UBlueprint::StaticClass();
}

UObject* UDamageOperationBlueprintFactory::FactoryCreateNew(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn)
{
	UClass* ParentClass = UDamageOperationBase::StaticClass();
	if (!FKismetEditorUtilities::CanCreateBlueprintOfClass(ParentClass))
	{
		return nullptr;
	}

	return FKismetEditorUtilities::CreateBlueprint(
		ParentClass,
		InParent,
		InName,
		BPTYPE_Normal,
		UBlueprint::StaticClass(),
		UBlueprintGeneratedClass::StaticClass(),
		NAME_None);
}

uint32 UDamageOperationBlueprintFactory::GetMenuCategories() const
{
	return FSagaStatsEditorModule::Get().GetDamagePipelineAssetCategory();
}

FText UDamageOperationBlueprintFactory::GetDisplayName() const
{
	return LOCTEXT("DamageOperationBlueprintFactoryDisplayName", "Damage Operation (Blueprint)");
}

#undef LOCTEXT_NAMESPACE
