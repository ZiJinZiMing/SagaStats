// Copyright 2022-2024 Mickael Daniel. All Rights Reserved.

#include "IGBAGameplayAttributeDataDetailsBase.h"

#include "AttributeSet.h"
#include "GBAEditorLog.h"
#include "GBAEditorSettings.h"
#include "IPropertyUtilities.h"
#include "PropertyHandle.h"
#include "ScopedTransaction.h"
#include "Kismet2/BlueprintEditorUtils.h"
#include "Utils/GBAEditorUtils.h"

#define LOCTEXT_NAMESPACE "GBAGameplayAttributeDataDetails"

IGBAGameplayAttributeDataDetailsBase::IGBAGameplayAttributeDataDetailsBase()
{
	GBA_EDITOR_LOG(VeryVerbose, TEXT("Construct IGBAGameplayAttributeDataDetailsBase ..."))
}

IGBAGameplayAttributeDataDetailsBase::~IGBAGameplayAttributeDataDetailsBase()
{
	GBA_EDITOR_LOG(VeryVerbose, TEXT("Destroyed IGBAGameplayAttributeDataDetailsBase ..."))
	PropertyBeingCustomized.Reset();
	BlueprintBeingCustomized.Reset();
	AttributeSetBeingCustomized.Reset();

	UGBAEditorSettings::GetMutable().OnSettingChanged().RemoveAll(this);
}

void IGBAGameplayAttributeDataDetailsBase::Initialize()
{
	UGBAEditorSettings::GetMutable().OnSettingChanged().AddSP(this, &IGBAGameplayAttributeDataDetailsBase::HandleSettingsChanged);
}

void IGBAGameplayAttributeDataDetailsBase::InitializeFromStructHandle(const TSharedRef<IPropertyHandle>& InStructPropertyHandle, IPropertyTypeCustomizationUtils& InStructCustomizationUtils)
{
	PropertyUtilities = InStructCustomizationUtils.GetPropertyUtilities();

	PropertyBeingCustomized = InStructPropertyHandle->GetProperty();
	check(PropertyBeingCustomized.IsValid());

	const UClass* OwnerClass = PropertyBeingCustomized->Owner.GetOwnerClass();
	check(OwnerClass);
	
	const UClass* OuterBaseClass = InStructPropertyHandle->GetOuterBaseClass();
	check(OuterBaseClass);

	// AttributeSetBeingCustomized = Cast<UAttributeSet>(OwnerClass->GetDefaultObject());
	AttributeSetBeingCustomized = Cast<UAttributeSet>(OuterBaseClass->GetDefaultObject());
	
	BlueprintBeingCustomized = UE::GBA::EditorUtils::GetBlueprintFromClass(OuterBaseClass);
}

TWeakFieldPtr<FProperty> IGBAGameplayAttributeDataDetailsBase::GetPropertyBeingCustomized() const
{
	return PropertyBeingCustomized;
}

TWeakObjectPtr<UBlueprint> IGBAGameplayAttributeDataDetailsBase::GetBlueprintBeingCustomized() const
{
	return BlueprintBeingCustomized;
}

TWeakObjectPtr<UAttributeSet> IGBAGameplayAttributeDataDetailsBase::GetAttributeSetBeingCustomized() const
{
	return AttributeSetBeingCustomized;
}

// ReSharper disable once CppMemberFunctionMayBeConst
// ReSharper disable CppParameterMayBeConstPtrOrRef
void IGBAGameplayAttributeDataDetailsBase::HandleSettingsChanged(UObject* InObject, FPropertyChangedEvent& InPropertyChangedEvent)
{
	GBA_EDITOR_LOG(VeryVerbose, TEXT("IGBAGameplayAttributeDataDetailsBase::HandleSettingsChanged - InObject: %s, Property: %s"), *GetNameSafe(InObject), *GetNameSafe(InPropertyChangedEvent.Property))

	if (const TSharedPtr<IPropertyUtilities> Utilities = PropertyUtilities.Pin())
	{
		GBA_EDITOR_LOG(VeryVerbose, TEXT("IGBAGameplayAttributeDataDetailsBase::HandleSettingsChanged - ForceRefresh InObject: %s, Property: %s"), *GetNameSafe(InObject), *GetNameSafe(InPropertyChangedEvent.Property))
		Utilities->ForceRefresh();
	}
}

TOptional<float> IGBAGameplayAttributeDataDetailsBase::OnGetBaseValue() const
{
	return LastSliderCommittedValue;
}

void IGBAGameplayAttributeDataDetailsBase::OnBeginSliderMovement()
{
	bIsUsingSlider = true;
}

void IGBAGameplayAttributeDataDetailsBase::OnEndSliderMovement(const float InValue)
{
	bIsUsingSlider = false;
	GBA_EDITOR_LOG(VeryVerbose, TEXT("End slider with %f (LastSliderCommittedValue: %f)"), InValue, LastSliderCommittedValue)
	SetValueWithTransaction(InValue);
}

// ReSharper disable once CppParameterNeverUsed
void IGBAGameplayAttributeDataDetailsBase::OnValueCommitted(const float InNewValue, ETextCommit::Type InCommitInfo)
{
	GBA_EDITOR_LOG(VeryVerbose, TEXT("OnValueCommitted: %f"), InNewValue)
	if (!bIsUsingSlider)
	{
		SetValueWithTransaction(InNewValue);
	}

	LastSliderCommittedValue = InNewValue;
}

void IGBAGameplayAttributeDataDetailsBase::SetValueWithTransaction(const float InNewValue) const
{
	FGameplayAttributeData* AttributeData = GetGameplayAttributeData();
	check(AttributeData);
	check(AttributeSetBeingCustomized.IsValid());
	check(BlueprintBeingCustomized.IsValid());

	FScopedTransaction Transaction(LOCTEXT("SetBaseValueProperty", "Set Attribute Data Base Value"));

	AttributeSetBeingCustomized->SetFlags(RF_Transactional);
	AttributeSetBeingCustomized->Modify();

	// Set the new value.
	AttributeData->SetBaseValue(InNewValue);
	AttributeData->SetCurrentValue(InNewValue);

	FBlueprintEditorUtils::MarkBlueprintAsStructurallyModified(BlueprintBeingCustomized.Get());
}

bool IGBAGameplayAttributeDataDetailsBase::IsCompactView()
{
	return UGBAEditorSettings::Get().bUseCompactView;
}

FText IGBAGameplayAttributeDataDetailsBase::GetHeaderBaseValueText() const
{
	const FGameplayAttributeData* AttributeData = GetGameplayAttributeData();
	check(AttributeData);
	
	return FText::Format(
		UGBAEditorSettings::Get().HeaderFormatText,
		FText::FromString(FString::Printf(TEXT("%.2f"), AttributeData->GetBaseValue())),
		FText::FromString(FString::Printf(TEXT("%.2f"), AttributeData->GetCurrentValue()))
	);
}

#undef LOCTEXT_NAMESPACE
