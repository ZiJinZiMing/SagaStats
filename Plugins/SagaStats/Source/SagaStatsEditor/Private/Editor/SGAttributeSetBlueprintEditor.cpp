/***************************************************************************************************************
* Plugin:       SagaStats
* Author:       Jinming Zhang
* Description:  SagaStats offers modular damage process and meter systems to support adaptable status management
****************************************************************************************************************/
#include "Editor/SGAttributeSetBlueprintEditor.h"

#include "AttributeSet.h"
#include "EdGraphSchema_K2.h"
#include "SGDelegates.h"
#include "SGEditorLog.h"
#include "AttributeSet/SGAttributeSetBlueprint.h"
#include "Details/Slate/SGNewAttributeViewModel.h"
#include "Details/Slate/SSGNewAttributeVariableWidget.h"
#include "Framework/MultiBox/MultiBoxBuilder.h"

#define LOCTEXT_NAMESPACE "SGAttributeSetBlueprintEditor"

/**
 * 构造函数 - 初始化AttributeSet蓝图编辑器实例
 * Constructor - Initializes the AttributeSet blueprint editor instance
 */
FSGAttributeSetBlueprintEditor::FSGAttributeSetBlueprintEditor()
{
}

/**
 * 析构函数 - 清理资源并关闭任何打开的窗口
 * Destructor - Cleans up resources and closes any open windows
 * 确保属性向导窗口被正确释放，防止内存泄漏
 * Ensures the attribute wizard window is properly released to prevent memory leaks
 */
FSGAttributeSetBlueprintEditor::~FSGAttributeSetBlueprintEditor()
{
	if (AttributeWizardWindow.IsValid())
	{
		AttributeWizardWindow.Reset();
	}
}

/**
 * 初始化AttributeSet编辑器 - 设置工具栏扩展和蓝图委托
 * Initialize AttributeSet Editor - Sets up toolbar extensions and blueprint delegates
 *
 * 此函数完成三个关键任务：
 * 1. 调用基类的蓝图编辑器初始化
 * 2. 在"Settings"按钮后添加自定义工具栏扩展（包含"Add Attribute"按钮）
 * 3. 为蓝图注册委托，以便在属性变化时接收通知
 *
 * This function performs three key tasks:
 * 1. Invokes base class blueprint editor initialization
 * 2. Adds custom toolbar extension after "Settings" button (includes "Add Attribute" button)
 * 3. Registers delegates on the blueprint to receive notifications when attributes change
 */
void FSGAttributeSetBlueprintEditor::InitAttributeSetEditor(const EToolkitMode::Type Mode, const TSharedPtr<IToolkitHost>& InitToolkitHost, const TArray<UBlueprint*>& InBlueprints, const bool bShouldOpenInDefaultsMode)
{
	// 初始化基础的蓝图编辑器功能 / Initialize base blueprint editor functionality
	InitBlueprintEditor(Mode, InitToolkitHost, InBlueprints, bShouldOpenInDefaultsMode);

	// 创建工具栏扩展器，将自定义按钮添加到编辑器工具栏
	// Create toolbar extender to add custom buttons to the editor toolbar
	const TSharedPtr<FExtender> ToolbarExtender = MakeShared<FExtender>();
	ToolbarExtender->AddToolBarExtension(
		TEXT("Settings"),
		EExtensionHook::After,
		GetToolkitCommands(),
		FToolBarExtensionDelegate::CreateSP(this, &FSGAttributeSetBlueprintEditor::FillToolbar)
	);

	// 应用工具栏扩展并重新生成UI / Apply toolbar extension and regenerate UI
	AddToolbarExtender(ToolbarExtender);
	RegenerateMenusAndToolbars();

	// 为第一个蓝图注册委托，以便响应属性变化事件
	// Register delegates on the first blueprint to respond to attribute change events
	if (InBlueprints.IsValidIndex(0))
	{
		if (USGAttributeSetBlueprint* Blueprint = Cast<USGAttributeSetBlueprint>(InBlueprints[0]))
		{
			Blueprint->RegisterDelegates();
		}
	}
}

/**
 * 编译蓝图 - 执行预编译广播、编译操作和后编译处理
 * Compile Blueprint - Executes pre-compile broadcast, compilation, and post-compile handling
 *
 * 编译流程：
 * 1. 广播预编译事件，通知其他系统（如GameplayEffect引用者）即将编译
 * 2. 调用基类编译方法执行实际的蓝图编译
 * 3. 重新聚焦窗口，防止USGEditorSubsystem重新打开GE引用时失去焦点
 *
 * Compilation flow:
 * 1. Broadcast pre-compile event to notify other systems (like GameplayEffect referencers)
 * 2. Invoke base class compile method to perform actual blueprint compilation
 * 3. Refocus window to prevent losing focus when USGEditorSubsystem reopens GE references
 */
void FSGAttributeSetBlueprintEditor::Compile()
{
	const UBlueprint* Blueprint = GetBlueprintObj();

	// 广播预编译事件，允许其他系统在编译前做准备（例如关闭依赖此AttributeSet的GameplayEffect编辑器）
	// Broadcast pre-compile event, allowing other systems to prepare (e.g., close GameplayEffect editors that depend on this AttributeSet)
	SG_EDITOR_NS_LOG(VeryVerbose, TEXT("PreCompile for %s"), *GetNameSafe(Blueprint))
	if (Blueprint)
	{
		if (const UPackage* Package = Blueprint->GetPackage())
		{
			FSGDelegates::OnPreCompile.Broadcast(Package->GetFName());
		}
	}

	// 执行实际的蓝图编译操作 / Perform the actual blueprint compilation
	FBlueprintEditor::Compile();
	SG_EDITOR_NS_LOG(VeryVerbose, TEXT("PostCompile for %s"), *GetNameSafe(Blueprint))

	// 重新聚焦此编辑器窗口，因为USGEditorSubsystem会关闭并重新打开GE引用者
	// 重新打开时总是聚焦最后一个GameplayEffect蓝图，此调用确保我们在点击Compile按钮后不会失去焦点
	// （但无法处理在Play时后台自动编译的情况）
	// Refocus this editor window because USGEditorSubsystem will close and reopen GE referencers
	// The reopen always focuses the last GameplayEffect BP, this call ensures we don't lose focus
	// when clicking the Compile button (but won't handle background compilation when hitting Play)
	FocusWindow();
}

/**
 * 初始化工具菜单上下文 - 设置菜单系统所需的上下文信息
 * Initialize Tool Menu Context - Sets up context information required by the menu system
 */
void FSGAttributeSetBlueprintEditor::InitToolMenuContext(FToolMenuContext& MenuContext)
{
	FBlueprintEditor::InitToolMenuContext(MenuContext);
}

/**
 * 获取工具包的唯一标识名称 - 用于在编辑器中识别此编辑器类型
 * Get Toolkit Unique Name - Used to identify this editor type within the editor
 */
FName FSGAttributeSetBlueprintEditor::GetToolkitFName() const
{
	return FName("SGAttributeSetBlueprintEditor");
}

/**
 * 获取工具包的基础显示名称 - 显示在编辑器标题栏中
 * Get Toolkit Base Display Name - Shown in the editor title bar
 */
FText FSGAttributeSetBlueprintEditor::GetBaseToolkitName() const
{
	return LOCTEXT("AttributeSetEditorAppLabel", "Saga Gameplay Attributes Editor");
}

/**
 * 获取工具包的工具提示文本 - 鼠标悬停在标签上时显示的信息
 * Get Toolkit Tooltip Text - Information shown when hovering over the tab
 */
FText FSGAttributeSetBlueprintEditor::GetToolkitToolTipText() const
{
	const UObject* EditingObject = GetEditingObject();

	check(EditingObject != nullptr);

	return GetToolTipTextForObject(EditingObject);
}

/**
 * 获取世界中心模式下的标签前缀 - 用于在Level Editor中嵌入编辑器时的标签命名
 * Get World-Centric Tab Prefix - Used for tab naming when embedded in Level Editor
 */
FString FSGAttributeSetBlueprintEditor::GetWorldCentricTabPrefix() const
{
	return TEXT("SGAttributeSetBlueprintEditor");
}

/**
 * 获取世界中心模式下的标签颜色 - 用于区分不同类型的编辑器标签
 * Get World-Centric Tab Color Scale - Used to distinguish different types of editor tabs
 */
FLinearColor FSGAttributeSetBlueprintEditor::GetWorldCentricTabColorScale() const
{
	return FLinearColor::White;
}

/**
 * 获取上次使用的Pin子类别对象 - 用于在创建新属性时保持用户的上一次选择
 * Get Last Pin Sub-Category Object - Used to persist user's previous selection when creating new attributes
 */
TWeakObjectPtr<UObject> FSGAttributeSetBlueprintEditor::GetLastPinSubCategoryObject() const
{
	return LastPinSubCategoryObject;
}

/**
 * 设置上次使用的Pin子类别对象 - 保存用户选择的数据类型以便下次使用
 * Set Last Pin Sub-Category Object - Saves user's selected data type for next use
 */
void FSGAttributeSetBlueprintEditor::SetLastPinSubCategoryObject(const TWeakObjectPtr<UObject>& InLastPinSubCategoryObject)
{
	LastPinSubCategoryObject = InLastPinSubCategoryObject;
}

/**
 * 获取上次的复制状态 - 返回上次创建属性时是否启用了网络复制
 * Get Last Replicated State - Returns whether network replication was enabled for the last created attribute
 */
bool FSGAttributeSetBlueprintEditor::GetLastReplicatedState() const
{
	return bLastReplicatedState;
}

/**
 * 设置上次的复制状态 - 保存复制设置以便下次创建属性时使用
 * Set Last Replicated State - Saves replication setting for use when creating the next attribute
 */
void FSGAttributeSetBlueprintEditor::SetLastReplicatedState(const bool bInLastReplicatedState)
{
	bLastReplicatedState = bInLastReplicatedState;
}

/**
 * 获取上次使用的变量名 - 返回上次创建的属性变量名
 * Get Last Used Variable Name - Returns the variable name of the last created attribute
 */
FString FSGAttributeSetBlueprintEditor::GetLastUsedVariableName() const
{
	return LastUsedVariableName;
}

/**
 * 设置上次使用的变量名 - 保存变量名以供参考或建议下一个名称
 * Set Last Used Variable Name - Saves variable name for reference or suggesting the next name
 */
void FSGAttributeSetBlueprintEditor::SetLastUsedVariableName(const FString& InLastUsedVariableName)
{
	LastUsedVariableName = InLastUsedVariableName;
}

/**
 * 填充工具栏 - 添加自定义按钮到编辑器工具栏
 * Fill Toolbar - Adds custom buttons to the editor toolbar
 *
 * 当前实现：
 * - 添加一个"Add Attribute"下拉按钮，用于创建新的Gameplay属性
 * - 未来计划：DataTable生成功能和C++代码生成功能（已注释）
 *
 * Current implementation:
 * - Adds an "Add Attribute" combo button for creating new Gameplay attributes
 * - Future plans: DataTable generation and C++ code generation features (commented out)
 */
void FSGAttributeSetBlueprintEditor::FillToolbar(FToolBarBuilder& InToolbarBuilder)
{
	// 开始"SagaAttributes"工具栏区段 / Begin the "SagaAttributes" toolbar section
	InToolbarBuilder.BeginSection(TEXT("SagaAttributes"));
	{
		// 添加"Add Attribute"下拉按钮，点击时显示属性创建面板
		// Add "Add Attribute" combo button that displays the attribute creation panel when clicked
		InToolbarBuilder.AddComboButton(
			FUIAction(),
			FOnGetContent::CreateSP(this, &FSGAttributeSetBlueprintEditor::GenerateToolbarMenu),
			LOCTEXT("ToolbarAddLabel", "Add Attribute"),
			LOCTEXT("ToolbarAddToolTip", "Create a new Attribute"),
			FSlateIcon(FAppStyle::GetAppStyleSetName(), "Icons.Plus"),
			false
		);

		/* TODO：DataTable功能和生成C++功能
		 * TODO: DataTable generation and C++ code generation features
		 *
		 * 计划功能1：从蓝图属性自动生成DataTable
		 * Planned feature 1: Automatically generate DataTable from Blueprint attributes
		InToolbarBuilder.AddToolBarButton(
			FUIAction(FExecuteAction::CreateSP(this, &FSGAttributeSetBlueprintEditor::CreateDataTableWindow)),
			NAME_None,
			LOCTEXT("ToolbarGenerateDataTableLabel", "Create DataTable"),
			LOCTEXT("ToolbarGenerateDataTableTooltip", "Automatically generate a DataTable from this Blueprint Attributes properties"),
			FSlateIcon(FAppStyle::GetAppStyleSetName(), "ClassIcon.DataTable")
		);

		 * 计划功能2：生成等效的C++代码
		 * Planned feature 2: Generate equivalent C++ code
		InToolbarBuilder.AddToolBarButton(
			FUIAction(FExecuteAction::CreateSP(this, &FSGAttributeSetBlueprintEditor::CreateAttributeWizard)),
			NAME_None,
			LOCTEXT("ToolbarGenerateCPPLabel", "Generate Equivalent C++"),
			LOCTEXT(
				"ToolbarGenerateCPPTooltip",
				"Provides a preview of what this class could look like in C++, "
				"and the ability to generate C++ header / source files from this Blueprint."
			),
			FSlateIcon(FSSEditorStyle::Get().GetStyleSetName(), "Icons.HeaderView")
		);
		*/

	}
	// 结束工具栏区段 / End toolbar section
	InToolbarBuilder.EndSection();
}

/**
 * 生成工具栏菜单 - 创建并返回"Add Attribute"下拉菜单的内容
 * Generate Toolbar Menu - Creates and returns the content for the "Add Attribute" dropdown menu
 *
 * 工作流程：
 * 1. 创建视图模型并用上次保存的设置初始化（变量名、复制状态、数据类型）
 * 2. 设置Pin类型，默认为FGameplayAttributeData结构体
 * 3. 创建属性创建控件并绑定取消/完成回调
 * 4. 将控件嵌入菜单构建器中
 *
 * Workflow:
 * 1. Create view model and initialize with last saved settings (variable name, replication state, data type)
 * 2. Set Pin type, defaulting to FGameplayAttributeData struct
 * 3. Create attribute creation widget and bind cancel/finish callbacks
 * 4. Embed widget in menu builder
 */
TSharedRef<SWidget> FSGAttributeSetBlueprintEditor::GenerateToolbarMenu()
{
	FMenuBuilder MenuBuilder(true, nullptr);

	// 创建视图模型，用于存储和管理属性创建的所有参数
	// Create view model to store and manage all parameters for attribute creation
	TSharedRef<FSGNewAttributeViewModel> ViewModel = MakeShared<FSGNewAttributeViewModel>();
	ViewModel->SetCustomizedBlueprint(GetBlueprintObj());
	ViewModel->SetVariableName(LastUsedVariableName);
	ViewModel->SetbIsReplicated(bLastReplicatedState);

	// 配置Pin类型，使用上次的选择或默认使用FGameplayAttributeData
	// Configure Pin type, using last selection or defaulting to FGameplayAttributeData
	FEdGraphPinType PinType;
	PinType.PinCategory = UEdGraphSchema_K2::PC_Struct;
	PinType.PinSubCategory = NAME_None;
	PinType.PinSubCategoryObject = LastPinSubCategoryObject.IsValid() ? LastPinSubCategoryObject.Get() : FGameplayAttributeData::StaticStruct();
	ViewModel->SetPinType(PinType);

	// 创建属性创建界面并绑定事件处理器
	// Create attribute creation UI and bind event handlers
	TSharedRef<SSGNewAttributeVariableWidget> Widget = SNew(SSGNewAttributeVariableWidget, ViewModel)
		.OnCancel_Static(&FSGAttributeSetBlueprintEditor::HandleAttributeWindowCancel)
		.OnFinish(this, &FSGAttributeSetBlueprintEditor::HandleAttributeWindowFinish);

	MenuBuilder.AddWidget(Widget, FText::GetEmpty());
	return MenuBuilder.MakeWidget();
}

/**
 * 处理属性窗口取消 - 用户点击取消按钮时的回调
 * Handle Attribute Window Cancel - Callback when user clicks the cancel button
 *
 * 此函数在用户取消属性创建时被调用，目前只进行视图模型有效性检查
 * 未来可以在此添加清理逻辑或记录用户行为
 *
 * This function is called when user cancels attribute creation, currently only validates view model
 * Future enhancements could include cleanup logic or user behavior tracking
 */
void FSGAttributeSetBlueprintEditor::HandleAttributeWindowCancel(const TSharedPtr<FSGNewAttributeViewModel>& InViewModel)
{
	check(InViewModel.IsValid());
}

/**
 * 处理属性窗口完成 - 用户确认创建属性时的回调
 * Handle Attribute Window Finish - Callback when user confirms attribute creation
 *
 * 工作流程：
 * 1. 从视图模型中提取所有用户设置（类型、名称、复制状态等）
 * 2. 保存这些设置作为下次创建属性时的默认值（提升用户体验）
 * 3. 调用静态函数AddMemberVariable实际创建蓝图成员变量
 *
 * Workflow:
 * 1. Extract all user settings from view model (type, name, replication state, etc.)
 * 2. Save these settings as defaults for next attribute creation (improves UX)
 * 3. Call static function AddMemberVariable to actually create the blueprint member variable
 */
void FSGAttributeSetBlueprintEditor::HandleAttributeWindowFinish(const TSharedPtr<FSGNewAttributeViewModel>& InViewModel)
{
	check(InViewModel.IsValid());

	// 保存用户的选择，作为下次创建属性时的默认值
	// Save user's selections as defaults for next attribute creation
	LastPinSubCategoryObject = InViewModel->GetPinType().PinSubCategoryObject;
	bLastReplicatedState = InViewModel->GetbIsReplicated();
	LastUsedVariableName = InViewModel->GetVariableName();

	// 实际创建成员变量，添加到蓝图中
	// Actually create the member variable and add it to the blueprint
	SSGNewAttributeVariableWidget::AddMemberVariable(
		GetBlueprintObj(),
		InViewModel->GetVariableName(),
		InViewModel->GetPinType(),
		InViewModel->GetDescription(),
		InViewModel->GetbIsReplicated()
	);
}
/*

void FSGAttributeSetBlueprintEditor::CreateAttributeWizard()
{
	const FSSAttributeWindowArgs WindowArgs;
	const FAssetData AssetData(GetBlueprintObj());
	if (!AttributeWizardWindow.IsValid())
	{
		AttributeWizardWindow = ISSScaffoldModule::Get().CreateAttributeWizard(AssetData, WindowArgs);
		AttributeWizardWindow->SetOnWindowClosed(FOnWindowClosed::CreateSP(this, &FSGAttributeSetBlueprintEditor::HandleAttributeWizardClosed));
	}
	else
	{
		AttributeWizardWindow->BringToFront();
	}
}

// ReSharper disable once CppParameterNeverUsed
void FSGAttributeSetBlueprintEditor::HandleAttributeWizardClosed(const TSharedRef<SWindow>& InWindow)
{
	if (AttributeWizardWindow.IsValid())
	{
		AttributeWizardWindow.Reset();
	}
}


void FSGAttributeSetBlueprintEditor::CreateDataTableWindow()
{
	if (!DataTableWindow.IsValid())
	{
		const FSSDataTableWindowArgs WindowArgs;
		DataTableWindow = ISSEditorModule::Get().CreateDataTableWindow(GetBlueprintObj(), WindowArgs);
		DataTableWindow->SetOnWindowClosed(FOnWindowClosed::CreateSP(this, &FSGAttributeSetBlueprintEditor::HandleDataTableWindowClosed));
	}
	else
	{
		DataTableWindow->BringToFront();
	}
}

// ReSharper disable once CppParameterNeverUsed
void FSGAttributeSetBlueprintEditor::HandleDataTableWindowClosed(const TSharedRef<SWindow>& InWindow)
{
	if (DataTableWindow.IsValid())
	{
		DataTableWindow.Reset();
	}
}
*/

#undef LOCTEXT_NAMESPACE
