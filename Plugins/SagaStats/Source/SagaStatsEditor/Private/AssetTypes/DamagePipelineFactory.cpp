// DamagePipelineFactory.cpp
#include "AssetTypes/DamagePipelineFactory.h"
#include "SagaStatsEditor.h"
#include "DamageFramework/DamagePipeline.h"

#define LOCTEXT_NAMESPACE "DamagePipelineFactory"

UDamagePipelineFactory::UDamagePipelineFactory()
{
	bCreateNew = true;
	bEditAfterNew = true;
	SupportedClass = UDamagePipeline::StaticClass();
}

UObject* UDamagePipelineFactory::FactoryCreateNew(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn)
{
	check(InClass->IsChildOf(UDamagePipeline::StaticClass()));
	return NewObject<UDamagePipeline>(InParent, InClass, InName, Flags);
}

uint32 UDamagePipelineFactory::GetMenuCategories() const
{
	return FSagaStatsEditorModule::Get().GetDamagePipelineAssetCategory();
}

FText UDamagePipelineFactory::GetDisplayName() const
{
	return LOCTEXT("DamagePipelineFactoryDisplayName", "Damage Pipeline");
}

#undef LOCTEXT_NAMESPACE
