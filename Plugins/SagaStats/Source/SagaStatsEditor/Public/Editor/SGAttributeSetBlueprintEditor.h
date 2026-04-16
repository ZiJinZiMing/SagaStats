/***************************************************************************************************************
* Plugin:       SagaStats
* Author:       Jinming Zhang
* Description:  SagaStats offers modular damage process and meter systems to support adaptable status management
****************************************************************************************************************/
#pragma once

#include "CoreMinimal.h"
#include "Editor/Kismet/Public/BlueprintEditor.h"

class FSGNewAttributeViewModel;

/**
 * Gameplay属性集蓝图编辑器 - 为SagaStats系统的属性集蓝图资产提供专用编辑器
 * Gameplay Attribute Set Blueprint Editor - Provides specialized editor for SagaStats Attribute Set Blueprint assets
 *
 * 该编辑器扩展了标准蓝图编辑器,为基于GAS(Gameplay Ability System)的属性集提供自定义工具栏和属性创建向导。
 * 它简化了游戏属性(如生命值、伤害等)的创建和管理流程,支持复制、数据类型选择等功能。
 * This editor extends the standard Blueprint editor to provide custom toolbar and attribute creation wizard for
 * GAS (Gameplay Ability System) based Attribute Sets. It streamlines the creation and management of game attributes
 * (such as health, damage, etc.), supporting replication, data type selection, and other features.
 *
 * 主要功能 / Key Features:
 * - 属性创建向导界面 / Attribute creation wizard interface
 * - 自定义工具栏操作 / Custom toolbar actions
 * - 记住上次使用的设置 / Remember last-used settings
 * - 与USGAttributeSetBlueprint资产类型集成 / Integration with USGAttributeSetBlueprint asset type
 */
class FSGAttributeSetBlueprintEditor : public FBlueprintEditor
{
public:
	FSGAttributeSetBlueprintEditor();
	virtual ~FSGAttributeSetBlueprintEditor() override;

	/**
	 * 初始化属性集蓝图编辑器 / Initialize the Attribute Set Blueprint editor
	 *
	 * 编辑指定的属性集蓝图资产,设置编辑器模式和工具包主机
	 * Edits the specified Attribute Set Blueprint asset(s), setting up editor mode and toolkit host
	 *
	 * @param	Mode					编辑器的资产编辑模式(独立或世界中心) / Asset editing mode for this editor (standalone or world-centric)
	 * @param	InitToolkitHost			当Mode为WorldCentric时,这是生成此编辑器的关卡编辑器实例 / When Mode is WorldCentric, this is the level editor instance to spawn this editor within
	 * @param	InBlueprints			要编辑的蓝图 / The blueprints to edit
	 * @param	bShouldOpenInDefaultsMode	如果为true,编辑器将在默认值编辑模式下打开 / If true, the editor will open in defaults editing mode
	 */
	void InitAttributeSetEditor(const EToolkitMode::Type Mode, const TSharedPtr<IToolkitHost>& InitToolkitHost, const TArray<UBlueprint*>& InBlueprints, bool bShouldOpenInDefaultsMode);

	//~ Begin FBlueprintEditor interface
	virtual void Compile() override;
	virtual void InitToolMenuContext(FToolMenuContext& MenuContext) override;
	//~ End FBlueprintEditor interface

	// IToolkit interface
	virtual FName GetToolkitFName() const override;
	virtual FText GetBaseToolkitName() const override;
	virtual FText GetToolkitToolTipText() const override;
	virtual FString GetWorldCentricTabPrefix() const override;
	virtual FLinearColor GetWorldCentricTabColorScale() const override;
	// End of IToolkit interface

	/** 获取上次使用的引脚子类别对象 / Get the last used pin sub-category object */
	TWeakObjectPtr<UObject> GetLastPinSubCategoryObject() const;

	/** 设置上次使用的引脚子类别对象 / Set the last used pin sub-category object */
	void SetLastPinSubCategoryObject(const TWeakObjectPtr<UObject>& InLastPinSubCategoryObject);

	/** 获取上次使用的复制状态 / Get the last used replicated state */
	bool GetLastReplicatedState() const;

	/** 设置上次使用的复制状态 / Set the last used replicated state */
	void SetLastReplicatedState(const bool bInLastReplicatedState);

	/** 获取上次使用的变量名 / Get the last used variable name */
	FString GetLastUsedVariableName() const;

	/** 设置上次使用的变量名 / Set the last used variable name */
	void SetLastUsedVariableName(const FString& InLastUsedVariableName);

private:
	/**
	 * 属性向导窗口的引用,确保同一时间只有一个实例
	 * Reference to the attribute wizard window to ensure only one instance at a time
	 */
	TSharedPtr<SWindow> AttributeWizardWindow;

	/**
	 * 数据表窗口的引用,确保同一时间只有一个实例
	 * Reference to the data table window to ensure only one instance at a time
	 */
	TSharedPtr<SWindow> DataTableWindow;

	/**
	 * 上次使用的引脚类型(用于添加新的属性变量),存储作为UObject的结构体(等同于PinType的PinSubCategoryObject)
	 * The last pin type used (to add a new Attribute variable), storing the struct as a UObject
	 * (equivalent of PinType PinSubCategoryObject)
	 *
	 * 用于在属性创建向导中记住用户上次选择的属性数据类型,提供更好的用户体验
	 * Used to remember the user's last selected attribute data type in the attribute creation wizard for better UX
	 */
	TWeakObjectPtr<UObject> LastPinSubCategoryObject;

	/**
	 * 上次使用的复制状态(通过工具栏或详情按钮添加新属性变量时)
	 * The last replicated state used (when adding a new Attribute variable via toolbar or details button)
	 *
	 * 默认为true,因为大多数游戏属性(如生命值、能量)需要在网络上复制
	 * Defaults to true as most game attributes (like health, energy) need to be replicated over the network
	 */
	bool bLastReplicatedState = true;

	/**
	 * 上次使用的变量名(通过工具栏或详情按钮添加新属性变量时)
	 * The last used variable name (when adding a new Attribute variable via toolbar or details button)
	 *
	 * 存储最近创建的属性名称,方便用户创建相关属性时参考
	 * Stores the most recently created attribute name for user reference when creating related attributes
	 */
	FString LastUsedVariableName;

	/**
	 * 为自定义操作构造工具栏小部件 / Construct toolbar widgets for custom actions
	 *
	 * 在编辑器工具栏中添加属性集特定的操作按钮和菜单项
	 * Adds Attribute Set-specific action buttons and menu items to the editor toolbar
	 */
	void FillToolbar(FToolBarBuilder& InToolbarBuilder);

	/**
	 * 为工具栏内容创建小部件 / Create widget for toolbar content
	 *
	 * 生成包含属性创建、数据表导入等选项的下拉菜单
	 * Generates a dropdown menu containing options for attribute creation, data table import, etc.
	 */
	TSharedRef<SWidget> GenerateToolbarMenu();

	/**
	 * 处理属性向导窗口取消操作 / Handle attribute wizard window cancel action
	 *
	 * 当用户取消属性创建时调用,清理视图模型和窗口状态
	 * Called when the user cancels attribute creation, cleaning up view model and window state
	 *
	 * @param InViewModel 包含用户输入数据的属性视图模型 / The attribute view model containing user input data
	 */
	static void HandleAttributeWindowCancel(const TSharedPtr<FSGNewAttributeViewModel>& InViewModel);

	/**
	 * 处理属性向导窗口完成操作 / Handle attribute wizard window finish action
	 *
	 * 当用户确认创建属性时调用,根据视图模型中的数据创建新的属性变量
	 * Called when the user confirms attribute creation, creating a new attribute variable based on view model data
	 *
	 * @param InViewModel 包含新属性配置的视图模型(名称、类型、复制状态等) / View model containing new attribute configuration (name, type, replication state, etc.)
	 */
	void HandleAttributeWindowFinish(const TSharedPtr<FSGNewAttributeViewModel>& InViewModel);
	
	/*

	void CreateAttributeWizard();
	void HandleAttributeWizardClosed(const TSharedRef<SWindow>& InWindow);

	void CreateDataTableWindow();
	void HandleDataTableWindowClosed(const TSharedRef<SWindow>& InWindow);
	*/

	
};
