// Copyright 2022-2024 Mickael Daniel. All Rights Reserved.

#include "Details/GBAGameplayAttributeDataClampedDetails.h"

#include "AttributeSet.h"
#include "DetailWidgetRow.h"
#include "GBAEditorLog.h"
#include "Details/GBAGameplayAttributeDataDetailsRow.h"
#include "IDetailChildrenBuilder.h"
#include "GBAAttributeSetBlueprintBase.h"
#include "Widgets/Text/STextBlock.h"

#define LOCTEXT_NAMESPACE "GBAGameplayAttributeDataDetails"

FGBAGameplayAttributeDataClampedDetails::FGBAGameplayAttributeDataClampedDetails()
{
	GBA_EDITOR_LOG(VeryVerbose, TEXT("Construct FGBAGameplayAttributeDataClampedDetails ..."))
}

FGBAGameplayAttributeDataClampedDetails::~FGBAGameplayAttributeDataClampedDetails()
{
	GBA_EDITOR_LOG(VeryVerbose, TEXT("Destroyed FGBAGameplayAttributeDataClampedDetails ..."))
	// if (MinValueRow.IsValid())
	// {
	// 	MinValueRow->OnValueChanged().RemoveAll(this);
	// 	MinValueRow.Reset();
	// }
	//
	// if (MaxValueRow.IsValid())
	// {
	// 	MaxValueRow->OnValueChanged().RemoveAll(this);
	// 	MaxValueRow.Reset();
	// }
	
	AttributeDataBeingCustomized.Reset();
}

TSharedRef<IPropertyTypeCustomization> FGBAGameplayAttributeDataClampedDetails::MakeInstance()
{
	TSharedRef<FGBAGameplayAttributeDataClampedDetails> Details = MakeShared<FGBAGameplayAttributeDataClampedDetails>();
	Details->Initialize();
	return Details;
}

void FGBAGameplayAttributeDataClampedDetails::InitializeFromStructHandle(const TSharedRef<IPropertyHandle>& InStructPropertyHandle, IPropertyTypeCustomizationUtils& InStructCustomizationUtils)
{
	IGBAGameplayAttributeDataDetailsBase::InitializeFromStructHandle(InStructPropertyHandle, InStructCustomizationUtils);

	if (!FGameplayAttribute::IsGameplayAttributeDataProperty(PropertyBeingCustomized.Get()))
	{
		return;
	}

	const FStructProperty* StructProperty = CastField<FStructProperty>(PropertyBeingCustomized.Get());
	check(StructProperty);

	if (FGameplayAttributeData* DataPtr = StructProperty->ContainerPtrToValuePtr<FGameplayAttributeData>(AttributeSetBeingCustomized.Get()))
	{
		const UStruct* Struct = StructProperty->Struct;
		if (Struct && Struct->IsChildOf(FGBAGameplayClampedAttributeData::StaticStruct()))
		{
			AttributeDataBeingCustomized = MakeShared<FGBAGameplayClampedAttributeData*>(static_cast<FGBAGameplayClampedAttributeData*>(DataPtr));
			if (AttributeDataBeingCustomized.IsValid())
			{
				const FGBAGameplayClampedAttributeData* Clamped = *AttributeDataBeingCustomized.Get();
				check(Clamped);
				GBA_EDITOR_LOG(VeryVerbose, TEXT("\t Clamped DataPtr -> %s: MinValue: %s, MaxValue: %s"), *GetNameSafe(PropertyBeingCustomized.Get()), *Clamped->MinValue.ToString(), *Clamped->MaxValue.ToString())
			}
		}
	}
}

FGameplayAttributeData* FGBAGameplayAttributeDataClampedDetails::GetGameplayAttributeData() const
{
	return GetGameplayClampedAttributeData();
}

void FGBAGameplayAttributeDataClampedDetails::CustomizeHeader(const TSharedRef<IPropertyHandle> InStructPropertyHandle, FDetailWidgetRow& InHeaderRow, IPropertyTypeCustomizationUtils& InStructCustomizationUtils)
{
	GBA_EDITOR_LOG(VeryVerbose, TEXT("FGBAGameplayAttributeDataClampedDetails::CustomizeHeader ..."))
	InitializeFromStructHandle(InStructPropertyHandle, InStructCustomizationUtils);

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
				.Text(this, &FGBAGameplayAttributeDataClampedDetails::GetHeaderBaseValueText)
			]
		];
}

void FGBAGameplayAttributeDataClampedDetails::CustomizeChildren(const TSharedRef<IPropertyHandle> InStructPropertyHandle, IDetailChildrenBuilder& InStructBuilder, IPropertyTypeCustomizationUtils& InStructCustomizationUtils)
{
	GBA_EDITOR_LOG(VeryVerbose, TEXT("FGBAGameplayAttributeDataClampedDetails::CustomizeChildren ..."))

	const FGBAGameplayClampedAttributeData* AttributeData = GetGameplayClampedAttributeData();
	if (!AttributeData)
	{
		return;
	}

	GBA_EDITOR_LOG(VeryVerbose, TEXT("AttributeDataBeingCustomized -> %s: %f"), *GetNameSafe(PropertyBeingCustomized.Get()), AttributeData->GetBaseValue())

	BaseValueRow = MakeShared<FGBAGameplayAttributeDataDetailsRow>(SharedThis(this), LOCTEXT("BaseValueLabel", "Base Value"), AttributeData->GetBaseValue());
	BaseValueRow->OnValueChanged().AddSP(this, &FGBAGameplayAttributeDataClampedDetails::HandleBaseValueChanged);
	BaseValueRow->CustomizeChildren(InStructPropertyHandle, InStructBuilder, InStructCustomizationUtils);

	const TSharedPtr<IPropertyHandle> MinValueHandle = InStructPropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FGBAGameplayClampedAttributeData, MinValue));
	if (MinValueHandle.IsValid())
	{
		IDetailPropertyRow& Row = InStructBuilder.AddProperty(MinValueHandle.ToSharedRef());
		Row.ShouldAutoExpand(true);
	}
	
	const TSharedPtr<IPropertyHandle> MaxValueHandle = InStructPropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FGBAGameplayClampedAttributeData, MaxValue));
	if (MaxValueHandle.IsValid())
	{
		IDetailPropertyRow& Row = InStructBuilder.AddProperty(MaxValueHandle.ToSharedRef());
		Row.ShouldAutoExpand(true);
	}
}

FGBAGameplayClampedAttributeData* FGBAGameplayAttributeDataClampedDetails::GetGameplayClampedAttributeData() const
{
	if (!AttributeDataBeingCustomized.IsValid())
	{
		return nullptr;
	}

	return *AttributeDataBeingCustomized.Get();
}

void FGBAGameplayAttributeDataClampedDetails::HandleBaseValueChanged(const float InValue) const
{
	FGBAGameplayClampedAttributeData* ClampedAttributeData = GetGameplayClampedAttributeData();
	check(ClampedAttributeData);
	
	// Set the new value.
	ClampedAttributeData->SetBaseValue(InValue);
	ClampedAttributeData->SetCurrentValue(InValue);
}

#undef LOCTEXT_NAMESPACE
