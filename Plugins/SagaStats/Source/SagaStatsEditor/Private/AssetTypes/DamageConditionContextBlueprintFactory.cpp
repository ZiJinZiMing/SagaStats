/***************************************************************************************************************
* Plugin:       SagaStats
* Author:       Claude(@JinmingZhang)
* Description:  SagaStats offers modular damage process and meter systems to support adaptable status management
****************************************************************************************************************/

// DamageConditionContextBlueprintFactory.cpp — Context 版 Condition 蓝图工厂
#include "AssetTypes/DamageConditionContextBlueprintFactory.h"
#include "SagaStatsEditor.h"
#include "DamagePipeline/DamageCondition_Context.h"
#include "Kismet2/KismetEditorUtilities.h"
#include "Engine/Blueprint.h"
#include "Engine/BlueprintGeneratedClass.h"

#define LOCTEXT_NAMESPACE "DamageConditionContextBlueprintFactory"

UDamageConditionContextBlueprintFactory::UDamageConditionContextBlueprintFactory()
{
	bCreateNew = true;
	bEditAfterNew = true;
	SupportedClass = UBlueprint::StaticClass();
}

UObject* UDamageConditionContextBlueprintFactory::FactoryCreateNew(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn)
{
	UClass* ParentClass = UDamageCondition_Context::StaticClass();
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

uint32 UDamageConditionContextBlueprintFactory::GetMenuCategories() const
{
	return FSagaStatsEditorModule::Get().GetDamagePipelineAssetCategory();
}

FText UDamageConditionContextBlueprintFactory::GetDisplayName() const
{
	return LOCTEXT("DamageConditionContextBlueprintFactoryDisplayName", "Damage Condition Context (Blueprint)");
}

#undef LOCTEXT_NAMESPACE
