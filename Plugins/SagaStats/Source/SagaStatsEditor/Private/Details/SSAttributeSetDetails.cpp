/******************************************************************************************
* Plugin:       SagaStats
* Author:       Jinming Zhang
* Description:  SagaStats is a status system that supports fully blueprintable attribute definitions and value calculations.
******************************************************************************************/
#include "Details/SSAttributeSetDetails.h"

#include "AttributeSet.h"
#include "DetailCategoryBuilder.h"
#include "DetailLayoutBuilder.h"
#include "EdGraphSchema_K2.h"
#include "SSEditorLog.h"
#include "Details/Slate/SSSNewAttributeVariableWidget.h"
#include "Details/Slate/SSSPositiveActionButton.h"
#include "Editor/SSBlueprintEditor.h"
#include "Engine/Blueprint.h"
#include "Slate/SSNewAttributeViewModel.h"
#include "Styling/SSAppStyle.h"
#include "Utils/SSEditorUtils.h"

#define LOCTEXT_NAMESPACE "FSSAttributeSetDetails"

TSharedRef<IDetailCustomization> FSSAttributeSetDetails::MakeInstance()
{
	return MakeShared<FSSAttributeSetDetails>();
}

void FSSAttributeSetDetails::CustomizeDetails(IDetailLayoutBuilder& InDetailLayout)
{
	AttributeBeingCustomized = UE::SS::EditorUtils::GetAttributeBeingCustomized(InDetailLayout);
	if (!AttributeBeingCustomized.IsValid())
	{
		return;
	}

	BlueprintBeingCustomized = UE::SS::EditorUtils::GetBlueprintFromClass(AttributeBeingCustomized->GetClass());
	if (!BlueprintBeingCustomized.IsValid())
	{
		return;
	}

	BlueprintEditorPtr = UE::SS::EditorUtils::FindBlueprintEditorForAsset(BlueprintBeingCustomized.Get());
	if (!BlueprintEditorPtr.IsValid())
	{
		return;
	}

	IDetailCategoryBuilder& Category = InDetailLayout.EditCategory("Default", LOCTEXT("DefaultsCategory", "Variables"));
	Category.RestoreExpansionState(false);

	const TSharedRef<SHorizontalBox> HeaderContentWidget = SNew(SHorizontalBox);
	HeaderContentWidget->AddSlot()
	[
		SNew(SHorizontalBox)
	];

	HeaderContentWidget->AddSlot()
	.AutoWidth()
	.VAlign(VAlign_Center)
	[
		SNew(SSSPositiveActionButton)
		.ToolTipText(LOCTEXT("NewInputPoseTooltip", "Create a new Attribute"))
		.Icon(FSSAppStyle::GetBrush("Icons.Plus"))
		.Text(LOCTEXT("AddNewLabel", "Add Attribute"))
		.OnGetMenuContent(this, &FSSAttributeSetDetails::CreateNewAttributeVariableWidget)
	];
	Category.HeaderContent(HeaderContentWidget);
}

TSharedRef<SWidget> FSSAttributeSetDetails::CreateNewAttributeVariableWidget()
{
	TSharedRef<FSSNewAttributeViewModel> ViewModel = MakeShared<FSSNewAttributeViewModel>();
	ViewModel->SetCustomizedBlueprint(BlueprintBeingCustomized);

	FEdGraphPinType PinType;
	PinType.PinCategory = UEdGraphSchema_K2::PC_Struct;
	PinType.PinSubCategory = NAME_None;
	PinType.PinSubCategoryObject = FGameplayAttributeData::StaticStruct();

	// Init from last saved variable states
	if (const TSharedPtr<FSSBlueprintEditor> BlueprintEditor = BlueprintEditorPtr.Pin()) 
	{
		ViewModel->SetVariableName(BlueprintEditor->GetLastUsedVariableName());
		ViewModel->SetbIsReplicated(BlueprintEditor->GetLastReplicatedState());
		if (BlueprintEditor->GetLastPinSubCategoryObject().IsValid())
		{
			PinType.PinSubCategoryObject = BlueprintEditor->GetLastPinSubCategoryObject();
		}
	}
	ViewModel->SetPinType(PinType);

	return SNew(SSSNewAttributeVariableWidget, ViewModel)
		.OnCancel_Static(&FSSAttributeSetDetails::HandleAttributeWindowCancel)
		.OnFinish(this, &FSSAttributeSetDetails::HandleAttributeWindowFinish);
}

void FSSAttributeSetDetails::HandleAttributeWindowCancel(const TSharedPtr<FSSNewAttributeViewModel>& InViewModel)
{
	check(InViewModel.IsValid());
	SS_EDITOR_LOG(Verbose, TEXT("Cancel -> %s"), *InViewModel->ToString())
}

void FSSAttributeSetDetails::HandleAttributeWindowFinish(const TSharedPtr<FSSNewAttributeViewModel>& InViewModel) const
{
	check(InViewModel.IsValid());
	SS_EDITOR_LOG(Verbose, TEXT("Finish -> %s"), *InViewModel->ToString())

	if (!BlueprintBeingCustomized.IsValid())
	{
		SS_EDITOR_LOG(Error, TEXT("Failed to add new variable to blueprint because BlueprintBeingCustomized is null"))
		return;
	}

	if (const TSharedPtr<FSSBlueprintEditor> BlueprintEditor = BlueprintEditorPtr.Pin()) 
	{
		BlueprintEditor->SetLastPinSubCategoryObject(InViewModel->GetPinType().PinSubCategoryObject);
		BlueprintEditor->SetLastReplicatedState(InViewModel->GetbIsReplicated());
	}

	SSSNewAttributeVariableWidget::AddMemberVariable(
		BlueprintBeingCustomized.Get(),
		InViewModel->GetVariableName(),
		InViewModel->GetPinType(),
		InViewModel->GetDescription(),
		InViewModel->GetbIsReplicated()
	);
}

#undef LOCTEXT_NAMESPACE
