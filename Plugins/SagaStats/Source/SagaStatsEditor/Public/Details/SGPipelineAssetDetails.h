// SGPipelineAssetDetails.h — DamagePipeline 的 Detail Customization（Build 按钮）
#pragma once

#include "CoreMinimal.h"
#include "IDetailCustomization.h"

class UDamagePipeline;

/**
 * 为 UDamagePipeline 的 Details 面板添加 Build 按钮。
 * 点击 Build 后根据 DamageRules 进行拓扑排序，烘焙 DAG 到 Asset 中。
 */
class FSGDamagePipelineDetails : public IDetailCustomization
{
public:
	static TSharedRef<IDetailCustomization> MakeInstance();
	virtual void CustomizeDetails(IDetailLayoutBuilder& InDetailLayout) override;

private:
	TWeakObjectPtr<UDamagePipeline> DamagePipeline;

	FReply OnBuildClicked();
};
