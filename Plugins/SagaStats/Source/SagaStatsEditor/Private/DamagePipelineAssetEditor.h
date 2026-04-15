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
	virtual ~FDamagePipelineAssetEditor();

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

	/** 动态返回 Build 按钮图标：Background + Overlay（Good 或 Unknown），随 bIsBaked 变化 */
	FSlateIcon GetBuildStatusIcon() const;
	FText GetBuildStatusTooltip() const;

	/** 响应被引用的 DamageRule（或其嵌套 Predicate/Condition）属性变化，置 bIsBaked = false */
	void OnObjectPropertyChanged(UObject* Object, struct FPropertyChangedEvent& PropertyChangedEvent);

	FDelegateHandle PropertyChangedHandle;

	/**
	 * 注册一个 ticker：下一帧读取所有节点的 Slate 真实尺寸，重新计算 Y 坐标，
	 * 解决 LayoutEngine 用估算尺寸导致的节点重叠问题。
	 */
	void ScheduleLayoutCorrection();

	/**
	 * 执行真实尺寸布局校正。
	 * @return 是否需要下一帧继续重试（如 Slate widget 尚未创建完成）
	 */
	bool ApplyRealSizeLayoutCorrection();

	static const FName GraphTabId;
	static const FName DetailsTabId;

	UDamagePipeline* Pipeline = nullptr;
	UDamagePipelineGraph* PipelineGraph = nullptr;

	TSharedPtr<SGraphEditor> GraphEditor;
	TSharedPtr<IDetailsView> DetailsView;
};
