// Copyright 2022-2024 Mickael Daniel. All Rights Reserved.


#include "Details/SSGameplayAttributePropertyDetails.h"

#include "AttributeSet.h"
#include "DetailWidgetRow.h"
#include "SSDelegates.h"
#include "SSEditorLog.h"
#include "IPropertyUtilities.h"
#include "Details/Slate/SSSGameplayAttributeWidget.h"

FSSGameplayAttributePropertyDetails::~FSSGameplayAttributePropertyDetails()
{
	FSSDelegates::OnRequestDetailsRefresh.RemoveAll(this);
}

TSharedRef<IPropertyTypeCustomization> FSSGameplayAttributePropertyDetails::MakeInstance()
{
	return MakeShared<FSSGameplayAttributePropertyDetails>();
}

// ReSharper disable once CppParameterMayBeConst
void FSSGameplayAttributePropertyDetails::CustomizeHeader(TSharedRef<IPropertyHandle> StructPropertyHandle, FDetailWidgetRow& HeaderRow, IPropertyTypeCustomizationUtils& StructCustomizationUtils)
{
	SS_EDITOR_LOG(Verbose, TEXT("FSSGameplayAttributePropertyDetails::CustomizeHeader ..."))

	const TSharedPtr<IPropertyUtilities> Utilities = StructCustomizationUtils.GetPropertyUtilities();
	FSSDelegates::OnRequestDetailsRefresh.AddSP(this, &FSSGameplayAttributePropertyDetails::HandleRequestRefresh, Utilities);

	// Can't use GET_MEMBER_NAME_CHECKED for those two props since they're private and requires adding this class as a friend class to FGameplayAttribute
	//
	// MyProperty = StructPropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FGameplayAttribute, Attribute));
	// OwnerProperty = StructPropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FGameplayAttribute, AttributeOwner));

	MyProperty = StructPropertyHandle->GetChildHandle(FName(TEXT("Attribute")));
	OwnerProperty = StructPropertyHandle->GetChildHandle(FName(TEXT("AttributeOwner")));
	NameProperty = StructPropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FGameplayAttribute, AttributeName));
	OwnerClassProperty = StructPropertyHandle->GetChildHandle(FName(TEXT("AttributeOwnerClass")));

	const FString& FilterMetaStr = StructPropertyHandle->GetProperty()->GetMetaData(TEXT("FilterMetaTag"));
	const bool bShowOnlyOwnedAttributes = StructPropertyHandle->GetProperty()->HasMetaData(TEXT("ShowOnlyOwnedAttributes"));
	const UClass* OuterBaseClass = StructPropertyHandle->GetOuterBaseClass();
	SS_EDITOR_LOG(
		VeryVerbose,
		TEXT("FSSGameplayAttributePropertyDetails::CustomizeHeader - OuterBaseClass: %s (bShowOnlyOwnedAttributed: %s)"),
		*GetNameSafe(OuterBaseClass),
		*LexToString(bShowOnlyOwnedAttributes)
	)

	FProperty* PropertyValue = nullptr;
	if (MyProperty.IsValid())
	{
		FProperty* ObjPtr = nullptr;
		MyProperty->GetValue(ObjPtr);
		PropertyValue = ObjPtr;
	}

	UClass* AttributeOwnerValue = nullptr;
	if (OwnerClassProperty.IsValid())
	{
		UObject* ObjPtr = nullptr;
		OwnerClassProperty->GetValue(ObjPtr);
		AttributeOwnerValue = Cast<UClass>(ObjPtr);
	}

	AttributeWidget = SNew(SSSGameplayAttributeWidget)
		.OnAttributeChanged(this, &FSSGameplayAttributePropertyDetails::OnAttributeChanged)
		.DefaultProperty(PropertyValue)
		.DefaultAttributeOwnerClass(AttributeOwnerValue)
		.FilterMetaData(FilterMetaStr)
		.ShowOnlyOwnedAttributes(bShowOnlyOwnedAttributes)
		.FilterClass(bShowOnlyOwnedAttributes ? OuterBaseClass : nullptr);

	HeaderRow.
	NameContent()
	[
		StructPropertyHandle->CreatePropertyNameWidget()
	]
	.ValueContent()
		.MinDesiredWidth(500)
		.MaxDesiredWidth(4096)
		[
			SNew(SHorizontalBox)
			+SHorizontalBox::Slot()
			.HAlign(HAlign_Fill)
			.Padding(0.f, 0.f, 2.f, 0.f)
			[
				AttributeWidget.ToSharedRef()
			]
		];
}

void FSSGameplayAttributePropertyDetails::CustomizeChildren(TSharedRef<IPropertyHandle> StructPropertyHandle, IDetailChildrenBuilder& StructBuilder, IPropertyTypeCustomizationUtils& StructCustomizationUtils)
{
}

void FSSGameplayAttributePropertyDetails::OnAttributeChanged(FProperty* SelectedAttribute, UClass* InAttributeOwnerClass) const
{
	if (MyProperty.IsValid())
	{
		MyProperty->SetValue(SelectedAttribute);
		OwnerClassProperty->SetValue(InAttributeOwnerClass);

		// When we set the attribute we should also set the owner and name info
		if (OwnerProperty.IsValid())
		{
			OwnerProperty->SetValue(SelectedAttribute ? SelectedAttribute->GetOwnerStruct() : nullptr);
		}

		if (NameProperty.IsValid())
		{
			FString AttributeName = TEXT("None");
			if (SelectedAttribute)
			{
				SelectedAttribute->GetName(AttributeName);
			}
			NameProperty->SetValue(AttributeName);
		}
	}
}

// ReSharper disable once CppMemberFunctionMayBeStatic
void FSSGameplayAttributePropertyDetails::HandleRequestRefresh(const TSharedPtr<IPropertyUtilities> InPropertyUtilities)
{
	SS_EDITOR_LOG(VeryVerbose, TEXT("FSSGameplayAttributePropertyDetails::HandleRequestRefresh ..."))
	if (InPropertyUtilities.IsValid())
	{
		InPropertyUtilities->ForceRefresh();
	}
}
