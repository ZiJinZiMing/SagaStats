/******************************************************************************************
* Plugin:       SagaStats
* Author:       Jinming Zhang
* Description:  SagaStats is a status system that supports fully blueprintable attribute definitions and value calculations.
******************************************************************************************/
#include "Editor/SSBlueprintEditor.h"

#include "AttributeSet.h"
#include "EdGraphSchema_K2.h"
#include "SSDelegates.h"
#include "SSEditorLog.h"
#include "SagaAttributeSetBlueprint.h"
#include "Details/Slate/SSNewAttributeViewModel.h"
#include "Details/Slate/SSSNewAttributeVariableWidget.h"
#include "Framework/MultiBox/MultiBoxBuilder.h"

#define LOCTEXT_NAMESPACE "SSBlueprintEditor"

FSSBlueprintEditor::FSSBlueprintEditor()
{
}

FSSBlueprintEditor::~FSSBlueprintEditor()
{
	if (AttributeWizardWindow.IsValid())
	{
		AttributeWizardWindow.Reset();
	}
}

void FSSBlueprintEditor::InitAttributeSetEditor(const EToolkitMode::Type Mode, const TSharedPtr<IToolkitHost>& InitToolkitHost, const TArray<UBlueprint*>& InBlueprints, const bool bShouldOpenInDefaultsMode)
{
	SS_EDITOR_LOG(Verbose, TEXT("FSSBlueprintEditor::InitAttributeSetEditor"))
	InitBlueprintEditor(Mode, InitToolkitHost, InBlueprints, bShouldOpenInDefaultsMode);

	const TSharedPtr<FExtender> ToolbarExtender = MakeShared<FExtender>();
	ToolbarExtender->AddToolBarExtension(
		TEXT("Settings"),
		EExtensionHook::After,
		GetToolkitCommands(),
		FToolBarExtensionDelegate::CreateSP(this, &FSSBlueprintEditor::FillToolbar)
	);

	AddToolbarExtender(ToolbarExtender);
	RegenerateMenusAndToolbars();

	if (InBlueprints.IsValidIndex(0))
	{
		if (USagaAttributeSetBlueprint* Blueprint = Cast<USagaAttributeSetBlueprint>(InBlueprints[0]))
		{
			Blueprint->RegisterDelegates();
		}
	}
}

void FSSBlueprintEditor::Compile()
{
	const UBlueprint* Blueprint = GetBlueprintObj();
	
	SS_EDITOR_LOG(VeryVerbose, TEXT("FSSBlueprintEditor::Compile - PreCompile for %s"), *GetNameSafe(Blueprint))
	if (Blueprint)
	{
		if (const UPackage* Package = Blueprint->GetPackage())
		{
			FSSDelegates::OnPreCompile.Broadcast(Package->GetFName());
		}
	}
	
	FBlueprintEditor::Compile();
	SS_EDITOR_LOG(VeryVerbose, TEXT("FSSBlueprintEditor::Compile - PostCompile for %s"), *GetNameSafe(Blueprint))

	// Bring toolkit back to front, cause USSEditorSubsystem will close any GE referencers and re-open
	// And the re-open will always focus the last Gameplay Effect BP, this focus window will happen after and make sure we don't loose focus
	// when we click the Compile button (but won't handle compile "in background" when hitting Play and some BP are automatically compiled by the editor)
	FocusWindow();
}

void FSSBlueprintEditor::InitToolMenuContext(FToolMenuContext& MenuContext)
{
	FBlueprintEditor::InitToolMenuContext(MenuContext);
}

FName FSSBlueprintEditor::GetToolkitFName() const
{
	return FName("SSBlueprintEditor");
}

FText FSSBlueprintEditor::GetBaseToolkitName() const
{
	return LOCTEXT("AttributeSetEditorAppLabel", "Gameplay Attributes Editor");
}

FText FSSBlueprintEditor::GetToolkitToolTipText() const
{
	const UObject* EditingObject = GetEditingObject();

	check(EditingObject != nullptr);

	return GetToolTipTextForObject(EditingObject);
}

FString FSSBlueprintEditor::GetWorldCentricTabPrefix() const
{
	return TEXT("BlueprintAttributeSetEditor");
}

FLinearColor FSSBlueprintEditor::GetWorldCentricTabColorScale() const
{
	return FLinearColor::White;
}

FString FSSBlueprintEditor::GetDocumentationLink() const
{
	return FBlueprintEditor::GetDocumentationLink(); // todo: point this at the correct documentation
}

TWeakObjectPtr<UObject> FSSBlueprintEditor::GetLastPinSubCategoryObject() const
{
	return LastPinSubCategoryObject;
}

void FSSBlueprintEditor::SetLastPinSubCategoryObject(const TWeakObjectPtr<UObject>& InLastPinSubCategoryObject)
{
	LastPinSubCategoryObject = InLastPinSubCategoryObject;
}

bool FSSBlueprintEditor::GetLastReplicatedState() const
{
	return bLastReplicatedState;
}

void FSSBlueprintEditor::SetLastReplicatedState(const bool bInLastReplicatedState)
{
	bLastReplicatedState = bInLastReplicatedState;
}

FString FSSBlueprintEditor::GetLastUsedVariableName() const
{
	return LastUsedVariableName;
}

void FSSBlueprintEditor::SetLastUsedVariableName(const FString& InLastUsedVariableName)
{
	LastUsedVariableName = InLastUsedVariableName;
}

void FSSBlueprintEditor::FillToolbar(FToolBarBuilder& InToolbarBuilder)
{
	InToolbarBuilder.BeginSection(TEXT("BlueprintAttributes"));
	{
		
		InToolbarBuilder.AddComboButton(
			FUIAction(),
			FOnGetContent::CreateSP(this, &FSSBlueprintEditor::GenerateToolbarMenu),
			LOCTEXT("ToolbarAddLabel", "Add Attribute"),
			LOCTEXT("ToolbarAddToolTip", "Create a new Attribute"),
			FSlateIcon(FAppStyle::GetAppStyleSetName(), "Icons.Plus"),
			false
		);
		
		/* TODO：DataTable功能和生成C++功能
		InToolbarBuilder.AddToolBarButton(
			FUIAction(FExecuteAction::CreateSP(this, &FSSBlueprintEditor::CreateDataTableWindow)),
			NAME_None,
			LOCTEXT("ToolbarGenerateDataTableLabel", "Create DataTable"),
			LOCTEXT("ToolbarGenerateDataTableTooltip", "Automatically generate a DataTable from this Blueprint Attributes properties"),
			FSlateIcon(FAppStyle::GetAppStyleSetName(), "ClassIcon.DataTable")
		);

		InToolbarBuilder.AddToolBarButton(
			FUIAction(FExecuteAction::CreateSP(this, &FSSBlueprintEditor::CreateAttributeWizard)),
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
	InToolbarBuilder.EndSection();
}

TSharedRef<SWidget> FSSBlueprintEditor::GenerateToolbarMenu()
{
	FMenuBuilder MenuBuilder(true, nullptr);

	TSharedRef<FSSNewAttributeViewModel> ViewModel = MakeShared<FSSNewAttributeViewModel>();
	ViewModel->SetCustomizedBlueprint(GetBlueprintObj());
	ViewModel->SetVariableName(LastUsedVariableName);
	ViewModel->SetbIsReplicated(bLastReplicatedState);

	FEdGraphPinType PinType;
	PinType.PinCategory = UEdGraphSchema_K2::PC_Struct;
	PinType.PinSubCategory = NAME_None;
	PinType.PinSubCategoryObject = LastPinSubCategoryObject.IsValid() ? LastPinSubCategoryObject.Get() : FGameplayAttributeData::StaticStruct();
	ViewModel->SetPinType(PinType);

	TSharedRef<SSSNewAttributeVariableWidget> Widget = SNew(SSSNewAttributeVariableWidget, ViewModel)
		.OnCancel_Static(&FSSBlueprintEditor::HandleAttributeWindowCancel)
		.OnFinish(this, &FSSBlueprintEditor::HandleAttributeWindowFinish);

	MenuBuilder.AddWidget(Widget, FText::GetEmpty());
	return MenuBuilder.MakeWidget();
}

void FSSBlueprintEditor::HandleAttributeWindowCancel(const TSharedPtr<FSSNewAttributeViewModel>& InViewModel)
{
	check(InViewModel.IsValid());
}

void FSSBlueprintEditor::HandleAttributeWindowFinish(const TSharedPtr<FSSNewAttributeViewModel>& InViewModel)
{
	check(InViewModel.IsValid());

	LastPinSubCategoryObject = InViewModel->GetPinType().PinSubCategoryObject;
	bLastReplicatedState = InViewModel->GetbIsReplicated();
	LastUsedVariableName = InViewModel->GetVariableName();

	SSSNewAttributeVariableWidget::AddMemberVariable(
		GetBlueprintObj(),
		InViewModel->GetVariableName(),
		InViewModel->GetPinType(),
		InViewModel->GetDescription(),
		InViewModel->GetbIsReplicated()
	);
}
/*

void FSSBlueprintEditor::CreateAttributeWizard()
{
	const FSSAttributeWindowArgs WindowArgs;
	const FAssetData AssetData(GetBlueprintObj());
	if (!AttributeWizardWindow.IsValid())
	{
		AttributeWizardWindow = ISSScaffoldModule::Get().CreateAttributeWizard(AssetData, WindowArgs);
		AttributeWizardWindow->SetOnWindowClosed(FOnWindowClosed::CreateSP(this, &FSSBlueprintEditor::HandleAttributeWizardClosed));
	}
	else
	{
		AttributeWizardWindow->BringToFront();
	}
}

// ReSharper disable once CppParameterNeverUsed
void FSSBlueprintEditor::HandleAttributeWizardClosed(const TSharedRef<SWindow>& InWindow)
{
	if (AttributeWizardWindow.IsValid())
	{
		AttributeWizardWindow.Reset();
	}
}


void FSSBlueprintEditor::CreateDataTableWindow()
{
	if (!DataTableWindow.IsValid())
	{
		const FSSDataTableWindowArgs WindowArgs;
		DataTableWindow = ISSEditorModule::Get().CreateDataTableWindow(GetBlueprintObj(), WindowArgs);
		DataTableWindow->SetOnWindowClosed(FOnWindowClosed::CreateSP(this, &FSSBlueprintEditor::HandleDataTableWindowClosed));
	}
	else
	{
		DataTableWindow->BringToFront();
	}
}

// ReSharper disable once CppParameterNeverUsed
void FSSBlueprintEditor::HandleDataTableWindowClosed(const TSharedRef<SWindow>& InWindow)
{
	if (DataTableWindow.IsValid())
	{
		DataTableWindow.Reset();
	}
}
*/

#undef LOCTEXT_NAMESPACE
