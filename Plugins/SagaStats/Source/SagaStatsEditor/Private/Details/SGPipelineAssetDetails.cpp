// SGPipelineAssetDetails.cpp — PipelineAsset Build 按钮实现
#include "Details/SGPipelineAssetDetails.h"
#include "DPUFramework/PipelineAsset.h"
#include "DetailLayoutBuilder.h"
#include "DetailCategoryBuilder.h"
#include "DetailWidgetRow.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Text/STextBlock.h"
#include "SGEditorLog.h"

TSharedRef<IDetailCustomization> FSGPipelineAssetDetails::MakeInstance()
{
	return MakeShared<FSGPipelineAssetDetails>();
}

void FSGPipelineAssetDetails::CustomizeDetails(IDetailLayoutBuilder& InDetailLayout)
{
	// 获取正在编辑的 PipelineAsset
	TArray<TWeakObjectPtr<UObject>> ObjectsBeingCustomized;
	InDetailLayout.GetObjectsBeingCustomized(ObjectsBeingCustomized);

	if (ObjectsBeingCustomized.Num() == 1)
	{
		PipelineAsset = Cast<UPipelineAsset>(ObjectsBeingCustomized[0].Get());
	}

	// 在 "Pipeline" 分类中添加 Build 按钮和状态显示
	IDetailCategoryBuilder& Category = InDetailLayout.EditCategory(
		TEXT("Pipeline Build"),
		FText::FromString(TEXT("Pipeline Build")),
		ECategoryPriority::Important);

	// Build 按钮
	Category.AddCustomRow(FText::FromString(TEXT("Build")))
		.NameContent()
		[
			SNew(STextBlock)
			.Text(FText::FromString(TEXT("DAG Topology")))
			.Font(IDetailLayoutBuilder::GetDetailFont())
		]
		.ValueContent()
		.MaxDesiredWidth(300.f)
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.Padding(2.f)
			[
				SNew(SButton)
				.Text(FText::FromString(TEXT("Build")))
				.ToolTipText(FText::FromString(TEXT("根据 DPU Definitions 的产销关系进行拓扑排序，烘焙 DAG 到 Asset 中")))
				.OnClicked(FOnClicked::CreateSP(this, &FSGPipelineAssetDetails::OnBuildClicked))
			]
		];

	// 状态行：显示 bIsBaked 和 DPU 数量
	Category.AddCustomRow(FText::FromString(TEXT("Status")))
		.NameContent()
		[
			SNew(STextBlock)
			.Text(FText::FromString(TEXT("Status")))
			.Font(IDetailLayoutBuilder::GetDetailFont())
		]
		.ValueContent()
		[
			SNew(STextBlock)
			.Font(IDetailLayoutBuilder::GetDetailFont())
			.Text_Lambda([this]() -> FText
			{
				if (!PipelineAsset.IsValid())
				{
					return FText::FromString(TEXT("(no asset)"));
				}
				UPipelineAsset* Asset = PipelineAsset.Get();
				FString Status = Asset->bIsBaked ? TEXT("Baked") : TEXT("Not Baked");
				return FText::FromString(FString::Printf(TEXT("%s | %d DPUs"),
					*Status, Asset->DPUDefinitions.Num()));
			})
			.ColorAndOpacity_Lambda([this]() -> FSlateColor
			{
				if (PipelineAsset.IsValid() && PipelineAsset->bIsBaked)
				{
					return FSlateColor(FLinearColor::Green);
				}
				return FSlateColor(FLinearColor(1.f, 0.6f, 0.f)); // 橙色
			})
		];
}

FReply FSGPipelineAssetDetails::OnBuildClicked()
{
	if (!PipelineAsset.IsValid())
	{
		SG_EDITOR_LOG(Warning, TEXT("PipelineAsset Build: 无有效 Asset"));
		return FReply::Handled();
	}

	UPipelineAsset* Asset = PipelineAsset.Get();

	SG_EDITOR_LOG(Log, TEXT("PipelineAsset Build 开始: %s (%d DPUs)"),
		*Asset->PipelineName.ToString(), Asset->DPUDefinitions.Num());

	FPipelineSortResult Result = Asset->Build();

	if (Result.bHasCycle)
	{
		SG_EDITOR_LOG(Error, TEXT("Build 失败: 检测到循环依赖"));
		for (const FString& Info : Result.CycleInfo)
		{
			SG_EDITOR_LOG(Error, TEXT("  %s"), *Info);
		}
	}
	else
	{
		SG_EDITOR_LOG(Log, TEXT("Build 成功: %d DPUs 已排序"), Result.SortedDPUs.Num());

		// 标记 Asset 为已修改，这样保存时会写入烘焙结果
		Asset->MarkPackageDirty();
	}

	return FReply::Handled();
}
