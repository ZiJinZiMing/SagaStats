/******************************************************************************************
* Plugin:       SagaStats
* Author:       Jinming Zhang
* Description:  SagaStats is a status system that supports fully blueprintable attribute definitions and value calculations.
******************************************************************************************/
#include "Details/SSGameplayAttributeDataDetails.h"

#include "AttributeSet.h"
#include "DetailWidgetRow.h"
#include "SSEditorLog.h"
#include "Details/SSGameplayAttributeDataDetailsRow.h"
#include "IDetailChildrenBuilder.h"
#include "Widgets/Text/STextBlock.h"

#define LOCTEXT_NAMESPACE "SSGameplayAttributeDataDetails"

FSSGameplayAttributeDataDetails::FSSGameplayAttributeDataDetails()
{
	SS_EDITOR_LOG(VeryVerbose, TEXT("Construct FSSGameplayAttributeDataDetails ..."))
}

FSSGameplayAttributeDataDetails::~FSSGameplayAttributeDataDetails()
{
	SS_EDITOR_LOG(VeryVerbose, TEXT("Destroyed FSSGameplayAttributeDataDetails ..."))
	AttributeDataBeingCustomized.Reset();
}

TSharedRef<IPropertyTypeCustomization> FSSGameplayAttributeDataDetails::MakeInstance()
{
	TSharedRef<FSSGameplayAttributeDataDetails> Details = MakeShared<FSSGameplayAttributeDataDetails>();
	Details->Initialize();
	return Details;
}

void FSSGameplayAttributeDataDetails::InitializeFromStructHandle(const TSharedRef<IPropertyHandle>& InStructPropertyHandle, IPropertyTypeCustomizationUtils& InStructCustomizationUtils)
{
	ISSGameplayAttributeDataDetailsBase::InitializeFromStructHandle(InStructPropertyHandle, InStructCustomizationUtils);

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

FGameplayAttributeData* FSSGameplayAttributeDataDetails::GetGameplayAttributeData() const
{
	if (!AttributeDataBeingCustomized.IsValid())
	{
		return nullptr;
	}

	return *AttributeDataBeingCustomized.Get();
}

void FSSGameplayAttributeDataDetails::CustomizeHeader(const TSharedRef<IPropertyHandle> InStructPropertyHandle, FDetailWidgetRow& InHeaderRow, IPropertyTypeCustomizationUtils& InStructCustomizationUtils)
{
	SS_EDITOR_LOG(VeryVerbose, TEXT("FSSGameplayAttributeDataDetails::CustomizeHeader ..."))
	InitializeFromStructHandle(InStructPropertyHandle, InStructCustomizationUtils);

	// Check if we want to display details in compact mode
	if (IsCompactView())
	{
		SS_EDITOR_LOG(VeryVerbose, TEXT("FSSGameplayAttributeDataDetails::CustomizeHeader bUseCompactView: true"))
		// Not adding anything to the row will make the header to not display
		return;
	}

	SS_EDITOR_LOG(VeryVerbose, TEXT("FSSGameplayAttributeDataDetails::CustomizeHeader bUseCompactView: false"))

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
				.Text(this, &FSSGameplayAttributeDataDetails::GetHeaderBaseValueText)
			]
		];
}

void FSSGameplayAttributeDataDetails::CustomizeChildren(const TSharedRef<IPropertyHandle> InStructPropertyHandle, IDetailChildrenBuilder& InStructBuilder, IPropertyTypeCustomizationUtils& InStructCustomizationUtils)
{
	SS_EDITOR_LOG(VeryVerbose, TEXT("FSSGameplayAttributeDataDetails::CustomizeChildren ..."))

	const FGameplayAttributeData* AttributeData = GetGameplayAttributeData();
	if (!AttributeData)
	{
		return;
	}

	SS_EDITOR_LOG(VeryVerbose, TEXT("AttributeDataBeingCustomized -> %s: %f"), *GetNameSafe(PropertyBeingCustomized.Get()), AttributeData->GetBaseValue())
	LastSliderCommittedValue = AttributeData->GetBaseValue();

	const FText NameText = IsCompactView() && PropertyBeingCustomized.IsValid() ? PropertyBeingCustomized->GetDisplayNameText() : LOCTEXT("BaseValueLabel", "Base Value");
	BaseValueRow = MakeShared<FSSGameplayAttributeDataDetailsRow>(SharedThis(this), NameText, AttributeData->GetBaseValue());
	BaseValueRow->OnValueChanged().AddSP(this, &FSSGameplayAttributeDataDetails::HandleBaseValueChanged);
	BaseValueRow->CustomizeChildren(InStructPropertyHandle, InStructBuilder, InStructCustomizationUtils);
}

void FSSGameplayAttributeDataDetails::HandleBaseValueChanged(const float InValue) const
{
	FGameplayAttributeData* AttributeData = GetGameplayAttributeData();
	check(AttributeData);
	
	// Set the new value.
	AttributeData->SetBaseValue(InValue);
	AttributeData->SetCurrentValue(InValue);
}

#undef LOCTEXT_NAMESPACE
