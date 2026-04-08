// SGPipelineAssetDetails.cpp — DamagePipeline Build 按钮实现
#include "Details/SGPipelineAssetDetails.h"
#include "DamageFramework/DamagePipeline.h"
#include "DetailLayoutBuilder.h"
#include "DetailCategoryBuilder.h"
#include "DetailWidgetRow.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Text/STextBlock.h"
#include "SGEditorLog.h"

TSharedRef<IDetailCustomization> FSGDamagePipelineDetails::MakeInstance()
{
	return MakeShared<FSGDamagePipelineDetails>();
}

void FSGDamagePipelineDetails::CustomizeDetails(IDetailLayoutBuilder& InDetailLayout)
{
	// 获取正在编辑的 DamagePipeline
	TArray<TWeakObjectPtr<UObject>> ObjectsBeingCustomized;
	InDetailLayout.GetObjectsBeingCustomized(ObjectsBeingCustomized);

	if (ObjectsBeingCustomized.Num() == 1)
	{
		DamagePipeline = Cast<UDamagePipeline>(ObjectsBeingCustomized[0].Get());
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
				.ToolTipText(FText::FromString(TEXT("根据 DamageRules 的产销关系进行拓扑排序，烘焙 DAG 到 Asset 中")))
				.OnClicked(FOnClicked::CreateSP(this, &FSGDamagePipelineDetails::OnBuildClicked))
			]
		];

	// 状态行：显示 bIsBaked 和 DamageRule 数量
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
				if (!DamagePipeline.IsValid())
				{
					return FText::FromString(TEXT("(no asset)"));
				}
				UDamagePipeline* Asset = DamagePipeline.Get();
				FString Status = Asset->bIsBaked ? TEXT("Baked") : TEXT("Not Baked");
				return FText::FromString(FString::Printf(TEXT("%s | %d Rules"),
					*Status, Asset->DamageRules.Num()));
			})
			.ColorAndOpacity_Lambda([this]() -> FSlateColor
			{
				if (DamagePipeline.IsValid() && DamagePipeline->bIsBaked)
				{
					return FSlateColor(FLinearColor::Green);
				}
				return FSlateColor(FLinearColor(1.f, 0.6f, 0.f)); // 橙色
			})
		];
}

FReply FSGDamagePipelineDetails::OnBuildClicked()
{
	if (!DamagePipeline.IsValid())
	{
		SG_EDITOR_LOG(Warning, TEXT("DamagePipeline Build: 无有效 Asset"));
		return FReply::Handled();
	}

	UDamagePipeline* Asset = DamagePipeline.Get();

	SG_EDITOR_LOG(Log, TEXT("DamagePipeline Build 开始: %s (%d Rules)"),
		*Asset->PipelineName.ToString(), Asset->DamageRules.Num());

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
		SG_EDITOR_LOG(Log, TEXT("Build 成功: %d Rules 已排序"), Result.SortedRules.Num());

		// 标记 Asset 为已修改，这样保存时会写入烘焙结果
		Asset->MarkPackageDirty();
	}

	return FReply::Handled();
}
