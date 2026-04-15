// DamageRuleFactory.cpp
#include "AssetTypes/DamageRuleFactory.h"
#include "SagaStatsEditor.h"
#include "DamageFramework/DamageRule.h"

#define LOCTEXT_NAMESPACE "DamageRuleFactory"

UDamageRuleFactory::UDamageRuleFactory()
{
	bCreateNew = true;
	bEditAfterNew = true;
	SupportedClass = UDamageRule::StaticClass();
}

UObject* UDamageRuleFactory::FactoryCreateNew(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn)
{
	check(InClass->IsChildOf(UDamageRule::StaticClass()));
	return NewObject<UDamageRule>(InParent, InClass, InName, Flags);
}

uint32 UDamageRuleFactory::GetMenuCategories() const
{
	return FSagaStatsEditorModule::Get().GetDamagePipelineAssetCategory();
}

FText UDamageRuleFactory::GetDisplayName() const
{
	return LOCTEXT("DamageRuleFactoryDisplayName", "Damage Rule");
}

#undef LOCTEXT_NAMESPACE
