// DamageEffectStructFactory.cpp
#include "AssetTypes/DamageEffectStructFactory.h"
#include "SagaStatsEditor.h"
#include "Engine/UserDefinedStruct.h"
#include "Kismet2/StructureEditorUtils.h"

#define LOCTEXT_NAMESPACE "DamageEffectStructFactory"

UDamageEffectStructFactory::UDamageEffectStructFactory()
{
	bCreateNew = true;
	bEditAfterNew = true;
	SupportedClass = UUserDefinedStruct::StaticClass();
}

UObject* UDamageEffectStructFactory::FactoryCreateNew(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn)
{
	// 复用 UE 内置的 User Defined Struct 创建工具
	return FStructureEditorUtils::CreateUserDefinedStruct(InParent, InName, Flags);
}

uint32 UDamageEffectStructFactory::GetMenuCategories() const
{
	return FSagaStatsEditorModule::Get().GetDamagePipelineAssetCategory();
}

FText UDamageEffectStructFactory::GetDisplayName() const
{
	return LOCTEXT("DamageEffectStructFactoryDisplayName", "Damage Effect Struct");
}

#undef LOCTEXT_NAMESPACE
