// DamagePipelineAssetEditor.h — DamagePipeline 的自定义资产编辑器（Details + Graph）
#pragma once

#include "CoreMinimal.h"
#include "Toolkits/AssetEditorToolkit.h"

class UDamagePipeline;
class UDamagePipelineGraph;
class SGraphEditor;
class IDetailsView;

/**
 * FDamagePipelineAssetEditor — DamagePipeline 的自定义编辑器。
 *
 * 双击 DamagePipeline 资产时打开，左侧 Details 面板、右侧 DAG 图表视图。
 * 工具栏提供 Build 按钮，触发拓扑排序烘焙并刷新图表。
 */
class FDamagePipelineAssetEditor : public FAssetEditorToolkit
{
public:
	void InitEditor(const EToolkitMode::Type Mode, const TSharedPtr<IToolkitHost>& Host, UDamagePipeline* InPipeline);

	// FAssetEditorToolkit
	virtual FName GetToolkitFName() const override { return FName("DamagePipelineEditor"); }
	virtual FText GetBaseToolkitName() const override { return NSLOCTEXT("DamagePipelineEditor", "AppLabel", "Damage Pipeline Editor"); }
	virtual FString GetWorldCentricTabPrefix() const override { return TEXT("DamagePipeline"); }
	virtual FLinearColor GetWorldCentricTabColorScale() const override { return FLinearColor(0.3f, 0.2f, 0.5f, 0.5f); }

	virtual void RegisterTabSpawners(const TSharedRef<FTabManager>& InTabManager) override;
	virtual void UnregisterTabSpawners(const TSharedRef<FTabManager>& InTabManager) override;

private:
	TSharedRef<SDockTab> SpawnTab_Details(const FSpawnTabArgs& Args);
	TSharedRef<SDockTab> SpawnTab_Graph(const FSpawnTabArgs& Args);

	void AddToolbarExtension(FToolBarBuilder& ToolBarBuilder);
	void OnBuild();

	static const FName GraphTabId;
	static const FName DetailsTabId;

	UDamagePipeline* Pipeline = nullptr;
	UDamagePipelineGraph* PipelineGraph = nullptr;

	TSharedPtr<SGraphEditor> GraphEditor;
	TSharedPtr<IDetailsView> DetailsView;
};
