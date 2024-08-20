// Copyright 2022-2024 Mickael Daniel. All Rights Reserved.

#include "GBAGameplayAttributeDataDetailsRow.h"

#include "AttributeSet.h"
#include "DetailLayoutBuilder.h"
#include "DetailWidgetRow.h"
#include "GBAEditorLog.h"
#include "IDetailChildrenBuilder.h"
#include "IGBAGameplayAttributeDataDetailsBase.h"
#include "ScopedTransaction.h"
#include "Kismet2/BlueprintEditorUtils.h"
#include "Widgets/Input/SNumericEntryBox.h"

#define LOCTEXT_NAMESPACE "GBAGameplayAttributeDataDetails"

FGBAGameplayAttributeDataDetailsRow::FGBAGameplayAttributeDataDetailsRow(const TWeakPtr<IGBAGameplayAttributeDataDetailsBase>& InDetailsOwner, const FText& InRowNameText, const float InInitialValue)
	: LastSliderCommittedValue(InInitialValue)
	, DetailsOwnerPtr(InDetailsOwner)
	, RowNameText(InRowNameText)
{
}

void FGBAGameplayAttributeDataDetailsRow::CustomizeChildren(TSharedRef<IPropertyHandle> InStructPropertyHandle, IDetailChildrenBuilder& InStructBuilder, IPropertyTypeCustomizationUtils& InStructCustomizationUtils)
{
	FDetailWidgetRow& Row = InStructBuilder.AddCustomRow(RowNameText);
	Row.NameContent()
	[
		SNew(STextBlock)
		.Text(RowNameText)
		.Font(IDetailLayoutBuilder::GetDetailFont())
	];

	Row.ValueContent()
	[
		SNew(SNumericEntryBox<float>)
		.Value(this, &FGBAGameplayAttributeDataDetailsRow::OnGetValue)
		// Because we have no lower or upper limit in MaxSliderValue, we need to "unspecify" the max value here,
		// otherwise the spinner has a limited range, with TNumericLimits<NumericType>::Max() as the MaxValue
		// and the spinning increment is huge
		.MinValue(TOptional<float>())
		.MaxValue(TOptional<float>())
		.MinSliderValue(TOptional<float>())
		.MaxSliderValue(TOptional<float>())
		.OnBeginSliderMovement(this, &FGBAGameplayAttributeDataDetailsRow::OnBeginSliderMovement)
		.OnEndSliderMovement(this, &FGBAGameplayAttributeDataDetailsRow::OnEndSliderMovement)
		.OnValueCommitted(this, &FGBAGameplayAttributeDataDetailsRow::OnValueCommitted)
		.OnValueChanged(this, &FGBAGameplayAttributeDataDetailsRow::OnValueCommitted, ETextCommit::Default)
		.AllowSpin(true)
	];
}

TOptional<float> FGBAGameplayAttributeDataDetailsRow::OnGetValue() const
{
	return LastSliderCommittedValue;
}

void FGBAGameplayAttributeDataDetailsRow::OnBeginSliderMovement()
{
	bIsUsingSlider = true;
}

void FGBAGameplayAttributeDataDetailsRow::OnEndSliderMovement(const float InValue)
{
	bIsUsingSlider = false;
	GBA_EDITOR_LOG(VeryVerbose, TEXT("End slider with %f (LastSliderCommittedValue: %f)"), InValue, LastSliderCommittedValue)
	SetValueWithTransaction(InValue);
}

// ReSharper disable once CppParameterNeverUsed
void FGBAGameplayAttributeDataDetailsRow::OnValueCommitted(const float InNewValue, ETextCommit::Type InCommitInfo)
{
	GBA_EDITOR_LOG(VeryVerbose, TEXT("OnValueCommitted: %f"), InNewValue)
	if (!bIsUsingSlider)
	{
		SetValueWithTransaction(InNewValue);
	}

	LastSliderCommittedValue = InNewValue;
}

void FGBAGameplayAttributeDataDetailsRow::SetValueWithTransaction(const float InNewValue) const
{
	const TSharedPtr<IGBAGameplayAttributeDataDetailsBase> DetailsOwner = DetailsOwnerPtr.Pin();
	check(DetailsOwner.IsValid());

	const FGameplayAttributeData* AttributeData = DetailsOwner->GetGameplayAttributeData();
	check(AttributeData);
	check(DetailsOwner->GetAttributeSetBeingCustomized().IsValid());

	FScopedTransaction Transaction(FText::Format(LOCTEXT("SetRowValueProperty", "Set Attribute Data {0}"), RowNameText));

	DetailsOwner->GetAttributeSetBeingCustomized()->SetFlags(RF_Transactional);
	DetailsOwner->GetAttributeSetBeingCustomized()->Modify();

	// Set the new value.
	OnValueChangedDelegate.Broadcast(InNewValue);

	if (DetailsOwner->GetBlueprintBeingCustomized().IsValid())
	{
		FBlueprintEditorUtils::MarkBlueprintAsStructurallyModified(DetailsOwner->GetBlueprintBeingCustomized().Get());
	}
}

#undef LOCTEXT_NAMESPACE