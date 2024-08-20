// Copyright 2022-2024 Mickael Daniel. All Rights Reserved.

#include "GBAGameplayAttributeDataDetails.h"

#include "AttributeSet.h"
#include "DetailWidgetRow.h"
#include "GBAEditorLog.h"
#include "GBAGameplayAttributeDataDetailsRow.h"
#include "IDetailChildrenBuilder.h"
#include "Widgets/Text/STextBlock.h"

#define LOCTEXT_NAMESPACE "GBAGameplayAttributeDataDetails"

FGBAGameplayAttributeDataDetails::FGBAGameplayAttributeDataDetails()
{
	GBA_EDITOR_LOG(VeryVerbose, TEXT("Construct FGBAGameplayAttributeDataDetails ..."))
}

FGBAGameplayAttributeDataDetails::~FGBAGameplayAttributeDataDetails()
{
	GBA_EDITOR_LOG(VeryVerbose, TEXT("Destroyed FGBAGameplayAttributeDataDetails ..."))
	AttributeDataBeingCustomized.Reset();
}

TSharedRef<IPropertyTypeCustomization> FGBAGameplayAttributeDataDetails::MakeInstance()
{
	TSharedRef<FGBAGameplayAttributeDataDetails> Details = MakeShared<FGBAGameplayAttributeDataDetails>();
	Details->Initialize();
	return Details;
}

void FGBAGameplayAttributeDataDetails::InitializeFromStructHandle(const TSharedRef<IPropertyHandle>& InStructPropertyHandle, IPropertyTypeCustomizationUtils& InStructCustomizationUtils)
{
	IGBAGameplayAttributeDataDetailsBase::InitializeFromStructHandle(InStructPropertyHandle, InStructCustomizationUtils);

	if (FGameplayAttribute::IsGameplayAttributeDataProperty(PropertyBeingCustomized.Get()))
	{
		const FStructProperty* StructProperty = CastField<FStructProperty>(PropertyBeingCustomized.Get());
		check(StructProperty);

		if (FGameplayAttributeData* DataPtr = StructProperty->ContainerPtrToValuePtr<FGameplayAttributeData>(AttributeSetBeingCustomized.Get()))
		{
			AttributeDataBeingCustomized = MakeShared<FGameplayAttributeData*>(DataPtr);
		}
	}
}

FGameplayAttributeData* FGBAGameplayAttributeDataDetails::GetGameplayAttributeData() const
{
	if (!AttributeDataBeingCustomized.IsValid())
	{
		return nullptr;
	}

	return *AttributeDataBeingCustomized.Get();
}

void FGBAGameplayAttributeDataDetails::CustomizeHeader(const TSharedRef<IPropertyHandle> InStructPropertyHandle, FDetailWidgetRow& InHeaderRow, IPropertyTypeCustomizationUtils& InStructCustomizationUtils)
{
	GBA_EDITOR_LOG(VeryVerbose, TEXT("FGBAGameplayAttributeDataDetails::CustomizeHeader ..."))
	InitializeFromStructHandle(InStructPropertyHandle, InStructCustomizationUtils);

	// Check if we want to display details in compact mode
	if (IsCompactView())
	{
		GBA_EDITOR_LOG(VeryVerbose, TEXT("FGBAGameplayAttributeDataDetails::CustomizeHeader bUseCompactView: true"))
		// Not adding anything to the row will make the header to not display
		return;
	}

	GBA_EDITOR_LOG(VeryVerbose, TEXT("FGBAGameplayAttributeDataDetails::CustomizeHeader bUseCompactView: false"))

	const FGameplayAttributeData* AttributeData = GetGameplayAttributeData();
	check(AttributeData);

	InHeaderRow.
		NameContent()
		[
			InStructPropertyHandle->CreatePropertyNameWidget()
		]
		.ValueContent()
		.MinDesiredWidth(500)
		.MaxDesiredWidth(4096)
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			  .HAlign(HAlign_Fill)
			  .Padding(0.f, 0.f, 2.f, 0.f)
			[
				SNew(STextBlock)
				.Text(this, &FGBAGameplayAttributeDataDetails::GetHeaderBaseValueText)
			]
		];
}

void FGBAGameplayAttributeDataDetails::CustomizeChildren(const TSharedRef<IPropertyHandle> InStructPropertyHandle, IDetailChildrenBuilder& InStructBuilder, IPropertyTypeCustomizationUtils& InStructCustomizationUtils)
{
	GBA_EDITOR_LOG(VeryVerbose, TEXT("FGBAGameplayAttributeDataDetails::CustomizeChildren ..."))

	const FGameplayAttributeData* AttributeData = GetGameplayAttributeData();
	if (!AttributeData)
	{
		return;
	}

	GBA_EDITOR_LOG(VeryVerbose, TEXT("AttributeDataBeingCustomized -> %s: %f"), *GetNameSafe(PropertyBeingCustomized.Get()), AttributeData->GetBaseValue())
	LastSliderCommittedValue = AttributeData->GetBaseValue();

	const FText NameText = IsCompactView() && PropertyBeingCustomized.IsValid() ? PropertyBeingCustomized->GetDisplayNameText() : LOCTEXT("BaseValueLabel", "Base Value");
	BaseValueRow = MakeShared<FGBAGameplayAttributeDataDetailsRow>(SharedThis(this), NameText, AttributeData->GetBaseValue());
	BaseValueRow->OnValueChanged().AddSP(this, &FGBAGameplayAttributeDataDetails::HandleBaseValueChanged);
	BaseValueRow->CustomizeChildren(InStructPropertyHandle, InStructBuilder, InStructCustomizationUtils);
}

void FGBAGameplayAttributeDataDetails::HandleBaseValueChanged(const float InValue) const
{
	FGameplayAttributeData* AttributeData = GetGameplayAttributeData();
	check(AttributeData);
	
	// Set the new value.
	AttributeData->SetBaseValue(InValue);
	AttributeData->SetCurrentValue(InValue);
}

#undef LOCTEXT_NAMESPACE
