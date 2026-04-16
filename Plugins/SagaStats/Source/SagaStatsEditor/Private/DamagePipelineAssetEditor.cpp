/***************************************************************************************************************
* Plugin:       SagaStats
* Author:       Claude(@JinmingZhang)
* Description:  SagaStats offers modular damage process and meter systems to support adaptable status management
****************************************************************************************************************/

// DamagePipelineAssetEditor.cpp — DamagePipeline 自定义资产编辑器实现

#include "DamagePipelineAssetEditor.h"

#include "DamagePipeline/DamagePipeline.h"
#include "DamagePipeline/DamageRule.h"
#include "DamagePipelineCommands.h"
#include "Graph/DamagePipelineGraph.h"
#include "Graph/DamagePipelineGraphSchema.h"
#include "Graph/DamagePipelineGraphNode.h"

#include "GraphEditor.h"
#include "SGraphPanel.h"
#include "SGraphNode.h"
#include "Graph/DamagePipelineLayoutConstants.h"
#include "PropertyEditorModule.h"
#include "Widgets/Docking/SDockTab.h"
#include "Framework/Commands/UICommandList.h"
#include "UObject/UObjectGlobals.h"
#include "Containers/Ticker.h"

#define LOCTEXT_NAMESPACE "DamagePipelineAssetEditor"

const FName FDamagePipelineAssetEditor::GraphTabId(TEXT("DPGraphTab"));
const FName FDamagePipelineAssetEditor::DetailsTabId(TEXT("DPDetailsTab"));

FDamagePipelineAssetEditor::~FDamagePipelineAssetEditor()
{
	if (PropertyChangedHandle.IsValid())
	{
		FCoreUObjectDelegates::OnObjectPropertyChanged.Remove(PropertyChangedHandle);
	}

	// 清理 Graph 对象——它 Outer 挂在 Pipeline DataAsset 上，
	// 不回收的话每次 InitEditor 都会新建一份泄漏到 Pipeline 的子对象图
	if (PipelineGraph && PipelineGraph->IsValidLowLevel())
	{
		PipelineGraph->MarkAsGarbage();
		PipelineGraph = nullptr;
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

	// 首次打开时需要真实尺寸布局（RebuildGraph 只创建数据层，位置留在 (0,0)）
	ScheduleLayoutCorrection();
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
	// LabelOverride 传 unset TAttribute → 回退到 FUICommandInfo 注册的 "Build"（本地化友好）
	ToolBarBuilder.AddToolBarButton(
		FDamagePipelineCommands::Get().Build,
		NAME_None,
		TAttribute<FText>(),
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

	// 重建后节点被重新 SNew，需要重新做真实尺寸校正
	ScheduleLayoutCorrection();
}

void FDamagePipelineAssetEditor::ScheduleLayoutCorrection()
{
	// WeakPtr 捕获以避免 Editor 提前销毁后悬空回调
	// 注意：局部名不能叫 WeakThis（会遮蔽 TSharedFromThis 的同名成员）
	TWeakPtr<FDamagePipelineAssetEditor> WeakSelf =
		StaticCastSharedRef<FDamagePipelineAssetEditor>(AsShared());

	// 共享重试计数器——mutable 捕获麻烦，用 SharedPtr 最简单
	TSharedRef<int32> RetryCountPtr = MakeShared<int32>(0);
	constexpr int32 MaxRetries = 10;

	FTSTicker::GetCoreTicker().AddTicker(
		FTickerDelegate::CreateLambda([WeakSelf, RetryCountPtr](float) -> bool
		{
			TSharedPtr<FDamagePipelineAssetEditor> Strong = WeakSelf.Pin();
			if (!Strong.IsValid())
			{
				return false; // Editor 已销毁，停止
			}

			if (!Strong->ApplyRealSizeLayoutCorrection())
			{
				return false; // 成功完成
			}

			// Slate widget 还没就绪，下一帧重试
			if (++(*RetryCountPtr) >= MaxRetries)
			{
				UE_LOG(LogTemp, Warning,
					TEXT("[DamagePipeline] Layout correction gave up after %d retries"),
					MaxRetries);
				return false;
			}
			return true;
		}),
		0.0f);
}

bool FDamagePipelineAssetEditor::ApplyRealSizeLayoutCorrection()
{
	if (!GraphEditor.IsValid() || !PipelineGraph)
	{
		return true; // 还没就绪，重试
	}

	SGraphPanel* Panel = GraphEditor->GetGraphPanel();
	if (!Panel)
	{
		return true;
	}

	// 按 SortIndex 拓扑序收集节点
	TArray<UDamagePipelineGraphNode*> SortedNodes;
	for (UEdGraphNode* EdNode : PipelineGraph->Nodes)
	{
		if (UDamagePipelineGraphNode* DPNode = Cast<UDamagePipelineGraphNode>(EdNode))
		{
			SortedNodes.Add(DPNode);
		}
	}
	SortedNodes.Sort([](const UDamagePipelineGraphNode& A, const UDamagePipelineGraphNode& B)
	{
		return A.SortIndex < B.SortIndex;
	});

	if (SortedNodes.Num() == 0)
	{
		return false; // 没节点，完成
	}

	// 每个 node 找它的 Slate widget。任一失败就全局重试。
	TArray<TSharedPtr<SGraphNode>> Widgets;
	Widgets.Reserve(SortedNodes.Num());
	for (UDamagePipelineGraphNode* Node : SortedNodes)
	{
		TSharedPtr<SGraphNode> Widget = Panel->GetNodeWidgetFromGuid(Node->NodeGuid);
		if (!Widget.IsValid())
		{
			return true; // widget 还没建，重试
		}
		Widgets.Add(Widget);
	}

	// 强制 SlatePrepass 让 GetDesiredSize 返回真实值
	// （首次 Tick 时 SGraphPanel 可能还没进入 Paint 阶段，DesiredSize 尚为 0）
	for (const TSharedPtr<SGraphNode>& Widget : Widgets)
	{
		Widget->SlatePrepass(1.0f);
	}

	// 预先统计每个节点的输入 Pin 数（X 方向用于决定"下一节点前的通道区"宽度）
	TArray<int32> InputCounts;
	InputCounts.Reserve(SortedNodes.Num());
	for (UDamagePipelineGraphNode* Node : SortedNodes)
	{
		int32 InputCount = 0;
		for (UEdGraphPin* Pin : Node->Pins)
		{
			if (Pin->Direction == EGPD_Input) ++InputCount;
		}
		InputCounts.Add(InputCount);
	}

	// 单遍遍历：用真实 width/height 同时累积 X 和 Y
	// 阶梯布局不变——每个节点 Y 单调递增，保证每个 Pin 全局唯一 Y
	double CurX = 0.0;
	double CurY = 0.0;
	bool bAnyChanged = false;
	for (int32 i = 0; i < SortedNodes.Num(); ++i)
	{
		UDamagePipelineGraphNode* Node = SortedNodes[i];
		const FVector2f DesiredSize = Widgets[i]->GetDesiredSize();
		const float RealW = FMath::Max(DesiredSize.X, 50.f);
		const float RealH = FMath::Max(DesiredSize.Y, 50.f);

		const int32 NewX = FMath::RoundToInt(CurX);
		const int32 NewY = FMath::RoundToInt(CurY);
		if (Node->NodePosX != NewX || Node->NodePosY != NewY)
		{
			Node->NodePosX = NewX;
			Node->NodePosY = NewY;
			bAnyChanged = true;
		}

		// Y：累加真实高度 + 固定间距
		CurY += RealH + DamagePipelineLayoutConstants::GapBetweenNodesY;

		// X：累加真实宽度 + BaseGap + 下一节点需要的输入通道区
		const int32 NextInputCount = InputCounts.IsValidIndex(i + 1) ? InputCounts[i + 1] : 0;
		CurX += RealW
			+ DamagePipelineLayoutConstants::BaseGap
			+ (double)NextInputCount * (double)DamagePipelineLayoutConstants::ChannelWidth;
	}

	if (bAnyChanged)
	{
		GraphEditor->NotifyGraphChanged();
	}

	return false; // 完成
}

#undef LOCTEXT_NAMESPACE
