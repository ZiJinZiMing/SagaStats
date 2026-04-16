/***************************************************************************************************************
* Plugin:       SagaStats
* Author:       Claude(@JinmingZhang)
* Description:  SagaStats offers modular damage process and meter systems to support adaptable status management
****************************************************************************************************************/

// DamageConditionBlueprintFactory.cpp
#include "AssetTypes/DamageConditionBlueprintFactory.h"
#include "SagaStatsEditor.h"
#include "DamagePipeline/DamageCondition.h"
#include "Kismet2/KismetEditorUtilities.h"
#include "Engine/Blueprint.h"
#include "Engine/BlueprintGeneratedClass.h"

#define LOCTEXT_NAMESPACE "DamageConditionBlueprintFactory"

UDamageConditionBlueprintFactory::UDamageConditionBlueprintFactory()
{
	bCreateNew = true;
	bEditAfterNew = true;
	SupportedClass = UBlueprint::StaticClass();
}

UObject* UDamageConditionBlueprintFactory::FactoryCreateNew(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn)
{
	UClass* ParentClass = UDamageCondition::StaticClass();
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

uint32 UDamageConditionBlueprintFactory::GetMenuCategories() const
{
	return FSagaStatsEditorModule::Get().GetDamagePipelineAssetCategory();
}

FText UDamageConditionBlueprintFactory::GetDisplayName() const
{
	return LOCTEXT("DamageConditionBlueprintFactoryDisplayName", "Damage Condition (Blueprint)");
}

#undef LOCTEXT_NAMESPACE
