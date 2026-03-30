// SGPipelineAssetDetails.h — PipelineAsset 的 Detail Customization（Build 按钮）
#pragma once

#include "CoreMinimal.h"
#include "IDetailCustomization.h"

class UPipelineAsset;

/**
 * 为 UPipelineAsset 的 Details 面板添加 Build 按钮。
 * 点击 Build 后根据 DPUDefinitions 进行拓扑排序，烘焙 DAG 到 Asset 中。
 */
class FSGPipelineAssetDetails : public IDetailCustomization
{
public:
	static TSharedRef<IDetailCustomization> MakeInstance();
	virtual void CustomizeDetails(IDetailLayoutBuilder& InDetailLayout) override;

private:
	TWeakObjectPtr<UPipelineAsset> PipelineAsset;

	FReply OnBuildClicked();
};
