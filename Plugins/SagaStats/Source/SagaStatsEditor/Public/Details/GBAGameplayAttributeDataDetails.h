// Copyright 2022-2024 Mickael Daniel. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "IGBAGameplayAttributeDataDetailsBase.h"

class FGBAGameplayAttributeDataDetailsRow;
class IPropertyHandle;
class IPropertyTypeCustomization;

/**
 * Details customization for FGameplayAttributeData.
 *
 * And ability to view / set BaseValue and CurrentValue (as DefaultValue)
 */
class FGBAGameplayAttributeDataDetails : public IGBAGameplayAttributeDataDetailsBase
{
public:
	FGBAGameplayAttributeDataDetails();
	virtual ~FGBAGameplayAttributeDataDetails() override;

	static TSharedRef<IPropertyTypeCustomization> MakeInstance();

	//~ Begin IGBAGameplayAttributeDataDetailsBase interface
	virtual void InitializeFromStructHandle(const TSharedRef<IPropertyHandle>& InStructPropertyHandle, IPropertyTypeCustomizationUtils& InStructCustomizationUtils) override;
	virtual FGameplayAttributeData* GetGameplayAttributeData() const override;
	//~ End IGBAGameplayAttributeDataDetailsBase interface

	//~ Begin IPropertyTypeCustomization interface
	virtual void CustomizeHeader(TSharedRef<IPropertyHandle> InStructPropertyHandle, FDetailWidgetRow& InHeaderRow, IPropertyTypeCustomizationUtils& InStructCustomizationUtils) override;
	virtual void CustomizeChildren(TSharedRef<IPropertyHandle> InStructPropertyHandle, IDetailChildrenBuilder& InStructBuilder, IPropertyTypeCustomizationUtils& InStructCustomizationUtils) override;
	//~ End IPropertyTypeCustomization interface

private:
	/** Keep track of the FGameplayAttributeData struct we are editing */
	TSharedPtr<FGameplayAttributeData*> AttributeDataBeingCustomized;

	TSharedPtr<FGBAGameplayAttributeDataDetailsRow> BaseValueRow;
	
	void HandleBaseValueChanged(float InValue) const;
};
