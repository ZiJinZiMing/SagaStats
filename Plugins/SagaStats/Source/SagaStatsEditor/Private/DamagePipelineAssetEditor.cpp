// DamagePipelineAssetEditor.cpp — DamagePipeline 自定义资产编辑器实现

#include "DamagePipelineAssetEditor.h"

#include "DamageFramework/DamagePipeline.h"
#include "DamageFramework/DamageRule.h"
#include "DamagePipelineCommands.h"
#include "Graph/DamagePipelineGraph.h"
#include "Graph/DamagePipelineGraphSchema.h"
#include "Graph/DamagePipelineGraphNode.h"

#include "GraphEditor.h"
#include "PropertyEditorModule.h"
#include "Widgets/Docking/SDockTab.h"
#include "Framework/Commands/UICommandList.h"
#include "UObject/UObjectGlobals.h"

#define LOCTEXT_NAMESPACE "DamagePipelineAssetEditor"

const FName FDamagePipelineAssetEditor::GraphTabId(TEXT("DPGraphTab"));
const FName FDamagePipelineAssetEditor::DetailsTabId(TEXT("DPDetailsTab"));

FDamagePipelineAssetEditor::~FDamagePipelineAssetEditor()
{
	if (PropertyChangedHandle.IsValid())
	{
		FCoreUObjectDelegates::OnObjectPropertyChanged.Remove(PropertyChangedHandle);
	}
}

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

	// --- 命令绑定（Build / F7） ---
	GetToolkitCommands()->MapAction(
		FDamagePipelineCommands::Get().Build,
		FExecuteAction::CreateSP(this, &FDamagePipelineAssetEditor::OnBuild));

	// --- 监听被引用的 DamageRule 属性变化（A/B/C/D 场景）---
	// Auto-Build 已经由上面的 RebuildGraph → Pipeline->Build() 覆盖，
	// 这里只负责"编辑器活着期间"被动响应改动
	PropertyChangedHandle = FCoreUObjectDelegates::OnObjectPropertyChanged.AddSP(
		this, &FDamagePipelineAssetEditor::OnObjectPropertyChanged);

	// --- 工具栏扩展 ---
	TSharedPtr<FExtender> Extender = MakeShared<FExtender>();
	Extender->AddToolBarExtension(
		"Asset",
		EExtensionHook::After,
		GetToolkitCommands(),
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
		FDamagePipelineCommands::Get().Build,
		NAME_None,
		FText::GetEmpty(),
		TAttribute<FText>::CreateSP(this, &FDamagePipelineAssetEditor::GetBuildStatusTooltip),
		TAttribute<FSlateIcon>::CreateSP(this, &FDamagePipelineAssetEditor::GetBuildStatusIcon));
}

FSlateIcon FDamagePipelineAssetEditor::GetBuildStatusIcon() const
{
	static const FName CompileStatusBackground("Blueprint.CompileStatus.Background");
	static const FName CompileStatusGood("Blueprint.CompileStatus.Overlay.Good");
	static const FName CompileStatusUnknown("Blueprint.CompileStatus.Overlay.Unknown");

	const bool bBaked = Pipeline && Pipeline->bIsBaked;
	return FSlateIcon(
		FAppStyle::GetAppStyleSetName(),
		CompileStatusBackground,
		NAME_None,
		bBaked ? CompileStatusGood : CompileStatusUnknown);
}

FText FDamagePipelineAssetEditor::GetBuildStatusTooltip() const
{
	const bool bBaked = Pipeline && Pipeline->bIsBaked;
	return bBaked
		? LOCTEXT("BuildTooltipGood", "Pipeline is built and up-to-date (F7 to rebuild)")
		: LOCTEXT("BuildTooltipDirty", "Pipeline needs build (F7 to build)");
}

void FDamagePipelineAssetEditor::OnObjectPropertyChanged(UObject* Object, FPropertyChangedEvent& PropertyChangedEvent)
{
	if (!Pipeline || !Object)
	{
		return;
	}

	// 向上遍历 Outer 链，找到第一个 UDamageRule。
	// 场景覆盖：
	//   - Object 本身就是 DamageRule（打开 DR_Guard.uasset 改属性）
	//   - Object 是 DamageRule 内嵌的 Predicate（改 Condition 树）
	//   - Object 是 Predicate 内嵌的 Condition（改原子条件的 EffectType）
	// 找到后检查是否在我们 Pipeline 引用的列表里，是则置 bIsBaked = false。
	for (UObject* Cur = Object; Cur; Cur = Cur->GetOuter())
	{
		if (UDamageRule* Rule = Cast<UDamageRule>(Cur))
		{
			if (Pipeline->DamageRules.Contains(Rule))
			{
				Pipeline->bIsBaked = false;
			}
			return;
		}
	}
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
