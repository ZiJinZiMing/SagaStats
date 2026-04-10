// DamagePipelineAssetEditor.cpp — DamagePipeline 自定义资产编辑器实现

#include "DamagePipelineAssetEditor.h"

#include "DamageFramework/DamagePipeline.h"
#include "Graph/DamagePipelineGraph.h"
#include "Graph/DamagePipelineGraphSchema.h"
#include "Graph/DamagePipelineGraphNode.h"

#include "GraphEditor.h"
#include "PropertyEditorModule.h"
#include "Widgets/Docking/SDockTab.h"

#define LOCTEXT_NAMESPACE "DamagePipelineAssetEditor"

const FName FDamagePipelineAssetEditor::GraphTabId(TEXT("DPGraphTab"));
const FName FDamagePipelineAssetEditor::DetailsTabId(TEXT("DPDetailsTab"));

void FDamagePipelineAssetEditor::InitEditor(const EToolkitMode::Type Mode, const TSharedPtr<IToolkitHost>& Host, UDamagePipeline* InPipeline)
{
	Pipeline = InPipeline;
	check(Pipeline);

	// --- 创建 Graph ---
	PipelineGraph = NewObject<UDamagePipelineGraph>(Pipeline, NAME_None, RF_Transactional);
	PipelineGraph->Schema = UDamagePipelineGraphSchema::StaticClass();
	PipelineGraph->RebuildGraph(Pipeline);

	// --- 创建 Details View ---
	FPropertyEditorModule& PropertyEditorModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");
	FDetailsViewArgs DetailsViewArgs;
	DetailsViewArgs.NameAreaSettings = FDetailsViewArgs::HideNameArea;
	DetailsViewArgs.bHideSelectionTip = true;
	DetailsView = PropertyEditorModule.CreateDetailView(DetailsViewArgs);
	DetailsView->SetObject(Pipeline);

	// --- 工具栏扩展 ---
	TSharedPtr<FExtender> Extender = MakeShared<FExtender>();
	Extender->AddToolBarExtension(
		"Asset",
		EExtensionHook::After,
		nullptr,
		FToolBarExtensionDelegate::CreateSP(this, &FDamagePipelineAssetEditor::AddToolbarExtension));
	AddToolbarExtender(Extender);

	// --- 布局 ---
	const TSharedRef<FTabManager::FLayout> Layout = FTabManager::NewLayout("DamagePipelineEditorLayout_v1")
		->AddArea
		(
			FTabManager::NewPrimaryArea()->SetOrientation(Orient_Horizontal)
			->Split
			(
				FTabManager::NewStack()
				->SetSizeCoefficient(0.3f)
				->AddTab(DetailsTabId, ETabState::OpenedTab)
			)
			->Split
			(
				FTabManager::NewStack()
				->SetSizeCoefficient(0.7f)
				->AddTab(GraphTabId, ETabState::OpenedTab)
			)
		);

	InitAssetEditor(Mode, Host, TEXT("DamagePipelineEditorApp"), Layout, true, true, Pipeline);
}

void FDamagePipelineAssetEditor::RegisterTabSpawners(const TSharedRef<FTabManager>& InTabManager)
{
	WorkspaceMenuCategory = InTabManager->AddLocalWorkspaceMenuCategory(
		LOCTEXT("WorkspaceMenu_DamagePipelineEditor", "Damage Pipeline Editor"));

	FAssetEditorToolkit::RegisterTabSpawners(InTabManager);

	InTabManager->RegisterTabSpawner(DetailsTabId,
		FOnSpawnTab::CreateSP(this, &FDamagePipelineAssetEditor::SpawnTab_Details))
		.SetDisplayName(LOCTEXT("DetailsTab", "Details"))
		.SetGroup(WorkspaceMenuCategory.ToSharedRef());

	InTabManager->RegisterTabSpawner(GraphTabId,
		FOnSpawnTab::CreateSP(this, &FDamagePipelineAssetEditor::SpawnTab_Graph))
		.SetDisplayName(LOCTEXT("GraphTab", "Graph"))
		.SetGroup(WorkspaceMenuCategory.ToSharedRef());
}

void FDamagePipelineAssetEditor::UnregisterTabSpawners(const TSharedRef<FTabManager>& InTabManager)
{
	FAssetEditorToolkit::UnregisterTabSpawners(InTabManager);
	InTabManager->UnregisterTabSpawner(DetailsTabId);
	InTabManager->UnregisterTabSpawner(GraphTabId);
}

TSharedRef<SDockTab> FDamagePipelineAssetEditor::SpawnTab_Graph(const FSpawnTabArgs& Args)
{
	check(PipelineGraph);

	GraphEditor = SNew(SGraphEditor)
		.IsEditable(false)
		.GraphToEdit(PipelineGraph);

	return SNew(SDockTab)
		.Label(LOCTEXT("GraphTabLabel", "Pipeline Graph"))
		[
			GraphEditor.ToSharedRef()
		];
}

TSharedRef<SDockTab> FDamagePipelineAssetEditor::SpawnTab_Details(const FSpawnTabArgs& Args)
{
	return SNew(SDockTab)
		.Label(LOCTEXT("DetailsTabLabel", "Details"))
		[
			DetailsView.ToSharedRef()
		];
}

void FDamagePipelineAssetEditor::AddToolbarExtension(FToolBarBuilder& ToolBarBuilder)
{
	ToolBarBuilder.AddToolBarButton(
		FUIAction(FExecuteAction::CreateSP(this, &FDamagePipelineAssetEditor::OnBuild)),
		NAME_None,
		LOCTEXT("Build", "Build"),
		LOCTEXT("BuildTooltip", "Build DAG topology"),
		FSlateIcon(FAppStyle::GetAppStyleSetName(), "Icons.Build"));
}

void FDamagePipelineAssetEditor::OnBuild()
{
	if (!Pipeline)
	{
		return;
	}

	Pipeline->Build();

	if (PipelineGraph)
	{
		PipelineGraph->RebuildGraph(Pipeline);
	}

	// 刷新 Graph Editor 显示
	if (GraphEditor.IsValid())
	{
		GraphEditor->NotifyGraphChanged();
	}

	// 刷新 Details 面板（bIsBaked 等状态变更）
	if (DetailsView.IsValid())
	{
		DetailsView->ForceRefresh();
	}
}

#undef LOCTEXT_NAMESPACE
