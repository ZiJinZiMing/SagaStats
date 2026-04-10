// DamagePipelineAssetTypeActions.h — DamagePipeline 资产类型注册
#pragma once

#include "CoreMinimal.h"
#include "AssetTypeActions_Base.h"

/**
 * FDamagePipelineAssetTypeActions — 注册 UDamagePipeline 的资产类型。
 *
 * 提供资产浏览器中的名称、颜色、分类，以及双击时打开自定义编辑器。
 */
class FDamagePipelineAssetTypeActions : public FAssetTypeActions_Base
{
public:
	// IAssetTypeActions
	virtual FText GetName() const override;
	virtual FColor GetTypeColor() const override;
	virtual UClass* GetSupportedClass() const override;
	virtual uint32 GetCategories() override;
	virtual void OpenAssetEditor(const TArray<UObject*>& InObjects, TSharedPtr<IToolkitHost> EditWithinLevelEditor = TSharedPtr<IToolkitHost>()) override;
};
