// DamagePipelineEditorSettings.h — DamagePipeline 编辑器显示偏好
#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "DamagePipelineEditorSettings.generated.h"

/** Condition 树多行排版的格式 */
UENUM()
enum class EDPConditionFormatStyle : uint8
{
	/** 纯缩进版：AND/OR 作为行首关键词 */
	IndentOnly UMETA(DisplayName = "Indent Only"),

	/** 带括号 + 缩进版：嵌套子树用 ( ) 单独占行 */
	BracketsAndIndent UMETA(DisplayName = "Brackets And Indent")
};

/**
 * UDamagePipelineEditorSettings — DamagePipeline Graph 编辑器的显示偏好
 *
 * Project Settings → Plugins → "DamagePipeline Editor" 下可配置。
 * 这里的所有字段都是"视图偏好"，不进 Pipeline 资产——不同开发者偏好不同而不污染资产 diff。
 */
UCLASS(config = EditorPerProjectUserSettings, meta = (DisplayName = "DamagePipeline Editor"))
class SAGASTATSEDITOR_API UDamagePipelineEditorSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	UDamagePipelineEditorSettings();

	// UDeveloperSettings
	virtual FName GetCategoryName() const override { return TEXT("Plugins"); }

	/** Condition 多行排版格式：纯缩进 或 带括号 */
	UPROPERTY(EditAnywhere, config, Category = "Display")
	EDPConditionFormatStyle ConditionFormatStyle;

	/** bReverse 时，NOT 是否占独立一行（否则作为前缀） */
	UPROPERTY(EditAnywhere, config, Category = "Display")
	bool bNotAsIndependentLine;
};
