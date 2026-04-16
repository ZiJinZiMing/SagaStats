/***************************************************************************************************************
* Plugin:       SagaStats
* Author:       Claude(@JinmingZhang)
* Description:  SagaStats offers modular damage process and meter systems to support adaptable status management
****************************************************************************************************************/

// DamagePipelineAssetTypeActions.cpp
#include "DamagePipelineAssetTypeActions.h"
#include "DamagePipelineAssetEditor.h"
#include "SagaStatsEditor.h"
#include "DamagePipeline/DamagePipeline.h"

#define LOCTEXT_NAMESPACE "DamagePipelineAssetTypeActions"

FText FDamagePipelineAssetTypeActions::GetName() const
{
	return LOCTEXT("Name", "Damage Pipeline");
}

FColor FDamagePipelineAssetTypeActions::GetTypeColor() const
{
	return FColor(108, 142, 191); // 蓝
}

UClass* FDamagePipelineAssetTypeActions::GetSupportedClass() const
{
	return UDamagePipeline::StaticClass();
}

uint32 FDamagePipelineAssetTypeActions::GetCategories()
{
	return FSagaStatsEditorModule::Get().GetDamagePipelineAssetCategory();
}

void FDamagePipelineAssetTypeActions::OpenAssetEditor(const TArray<UObject*>& InObjects, TSharedPtr<IToolkitHost> EditWithinLevelEditor)
{
	EToolkitMode::Type Mode = EditWithinLevelEditor.IsValid() ? EToolkitMode::WorldCentric : EToolkitMode::Standalone;

	for (UObject* Obj : InObjects)
	{
		if (UDamagePipeline* Pipeline = Cast<UDamagePipeline>(Obj))
		{
			TSharedRef<FDamagePipelineAssetEditor> Editor = MakeShared<FDamagePipelineAssetEditor>();
			Editor->InitEditor(Mode, EditWithinLevelEditor, Pipeline);
		}
	}
}

#undef LOCTEXT_NAMESPACE
