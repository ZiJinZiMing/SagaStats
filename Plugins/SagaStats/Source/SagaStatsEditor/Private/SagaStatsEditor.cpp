#include "SagaStatsEditor.h"
#include "AssetToolsModule.h"
#include "GBAEditorLog.h"
#include "PropertyEditorModule.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "Details/GBAGameplayAttributeDataClampedDetails.h"
#include "Details/GBAGameplayAttributeDataDetails.h"
#include "Details/GBAGameplayAttributePropertyDetails.h"
#include "Editor/GBAGraphPanelPinFactory.h"
#include "Framework/Application/SlateApplication.h"
#include "Interfaces/IMainFrameModule.h"
#include "Misc/EngineVersionComparison.h"
#define LOCTEXT_NAMESPACE "FSagaStatsEditorModule"

void FSagaStatsEditorModule::StartupModule()
{
	// Module is set to kick in in PreLoadingScreen
	// so that it starts right before GameplayAbilitiesEditorModule (which is PreDefault)
	//
	// Mainly to hook in our own GraphPanelPinFactory to return our version of SGraphPing widget, since we can't really unregister
	// the visual pin factory of Gameplay Abilities Editor module from outside modules.
	//
	// Registering ours earlier so that editor considers it before evaluating the default one (we need to expose Attributes added in BP for K2 Nodes)
	//
	GBA_EDITOR_LOG(Verbose, TEXT("FGBAEditorModule::StartupModule"))


	// Every other logic that would have happen in here is delayed to OnPostEngineInit
	FCoreDelegates::OnPostEngineInit.AddRaw(this, &FSagaStatsEditorModule::OnPostEngineInit);

	// Register factories for pins and nodes
	//
	// That is for K2 nodes with FGameplayAttribute pins, like GetFloatAttributeBase from ASC
	GameplayAbilitiesGraphPanelPinFactory = MakeShared<FGBAGraphPanelPinFactory>();
	FEdGraphUtilities::RegisterVisualPinFactory(GameplayAbilitiesGraphPanelPinFactory);
}

void FSagaStatsEditorModule::ShutdownModule()
{
	GBA_EDITOR_LOG(Verbose, TEXT("FGBAEditorModule::ShutdownModule"))

	FCoreDelegates::OnPostEngineInit.RemoveAll(this);
	

	// Unregister customizations
	if (FModuleManager::Get().IsModuleLoaded(TEXT("PropertyEditor")))
	{
		FPropertyEditorModule& PropertyModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>(TEXT("PropertyEditor"));
		PropertyModule.UnregisterCustomPropertyTypeLayout(TEXT("GameplayAttribute"));
		PropertyModule.UnregisterCustomPropertyTypeLayout(TEXT("GameplayAttributeData"));
		PropertyModule.UnregisterCustomPropertyTypeLayout(TEXT("GBAGameplayClampedAttributeData"));

		// PropertyModule.UnregisterCustomClassLayout(TEXT("GBAAttributeSetBlueprintBase"));
	}

	// Unregister graph factories
	if (GameplayAbilitiesGraphPanelPinFactory.IsValid())
	{
		FEdGraphUtilities::UnregisterVisualPinFactory(GameplayAbilitiesGraphPanelPinFactory);
		GameplayAbilitiesGraphPanelPinFactory.Reset();
	}
}

void FSagaStatsEditorModule::PreloadAssetsByClass(UClass* InClass) const
{
	//TODO:
}

void FSagaStatsEditorModule::OnPostEngineInit()
{
	// Detail Customizations
	{
		FPropertyEditorModule& PropertyModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>(TEXT("PropertyEditor"));

		// Unregister GAS default customization for Gameplay Attributes and Sets
		// PropertyModule.UnregisterCustomClassLayout(TEXT("AttributeSet"));
		PropertyModule.UnregisterCustomPropertyTypeLayout("GameplayAttribute");

		// And register our own customizations
		PropertyModule.RegisterCustomPropertyTypeLayout(TEXT("GameplayAttribute"), FOnGetPropertyTypeCustomizationInstance::CreateStatic(&FGBAGameplayAttributePropertyDetails::MakeInstance));
		PropertyModule.RegisterCustomPropertyTypeLayout(TEXT("GameplayAttributeData"), FOnGetPropertyTypeCustomizationInstance::CreateStatic(&FGBAGameplayAttributeDataDetails::MakeInstance));
		PropertyModule.RegisterCustomPropertyTypeLayout(TEXT("GBAGameplayClampedAttributeData"), FOnGetPropertyTypeCustomizationInstance::CreateStatic(&FGBAGameplayAttributeDataClampedDetails::MakeInstance));
		
		// PropertyModule.RegisterCustomClassLayout(TEXT("GBAAttributeSetBlueprintBase"), FOnGetDetailCustomizationInstance::CreateStatic(&FGBAAttributeSetDetails::MakeInstance));
	}
}

#undef LOCTEXT_NAMESPACE
    
IMPLEMENT_MODULE(FSagaStatsEditorModule, SagaStatsEditor)