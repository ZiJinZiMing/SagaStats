// Copyright 2022-2024 Mickael Daniel. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "IGBAGameplayAttributeDataDetailsBase.h"

class FGBAGameplayAttributeDataDetailsRow;
struct FGBAGameplayClampedAttributeData;

/**
 * Details customization for FGBAGameplayClampedAttributeData.
 *
 * And ability to view / set BaseValue and CurrentValue (as DefaultValue)
 */
class FGBAGameplayAttributeDataClampedDetails : public IGBAGameplayAttributeDataDetailsBase
{
public:
	FGBAGameplayAttributeDataClampedDetails();
	virtual ~FGBAGameplayAttributeDataClampedDetails() override;

	static TSharedRef<IPropertyTypeCustomization> MakeInstance();

	//~ Begin FGBAGameplayAttributeDataClampedDetailsBase interface
	virtual void InitializeFromStructHandle(const TSharedRef<IPropertyHandle>& InStructPropertyHandle, IPropertyTypeCustomizationUtils& InStructCustomizationUtils) override;
	virtual FGameplayAttributeData* GetGameplayAttributeData() const override;
	//~ End FGBAGameplayAttributeDataClampedDetailsBase interface

	//~ Begin IPropertyTypeCustomization interface
	virtual void CustomizeHeader(TSharedRef<IPropertyHandle> InStructPropertyHandle, FDetailWidgetRow& InHeaderRow, IPropertyTypeCustomizationUtils& InStructCustomizationUtils) override;
	virtual void CustomizeChildren(TSharedRef<IPropertyHandle> InStructPropertyHandle, IDetailChildrenBuilder& InStructBuilder, IPropertyTypeCustomizationUtils& InStructCustomizationUtils) override;
	//~ End IPropertyTypeCustomization interface

	FGBAGameplayClampedAttributeData* GetGameplayClampedAttributeData() const;

private:
	/** Keep track of the FGBAGameplayClampedAttributeData struct we are editing */
	TSharedPtr<FGBAGameplayClampedAttributeData*> AttributeDataBeingCustomized;

	TSharedPtr<FGBAGameplayAttributeDataDetailsRow> BaseValueRow;

	void HandleBaseValueChanged(float InValue) const;
};
