/******************************************************************************************
* Plugin:       SagaStats
* Author:       Jinming Zhang
* Description:  SagaStats is a status system that supports fully blueprintable attribute definitions and value calculations.
******************************************************************************************/
#pragma once

#include "CoreMinimal.h"

#include "ISSGameplayAttributeDataDetailsBase.h"

class FSSGameplayAttributeDataDetailsRow;
struct FSagaClampedAttributeData;

/**
 * Details customization for FSagaClampedAttributeData.
 *
 * And ability to view / set BaseValue and CurrentValue (as DefaultValue)
 */
class FSSGameplayAttributeDataClampedDetails : public ISSGameplayAttributeDataDetailsBase
{
public:
	FSSGameplayAttributeDataClampedDetails();
	virtual ~FSSGameplayAttributeDataClampedDetails() override;

	static TSharedRef<IPropertyTypeCustomization> MakeInstance();

	//~ Begin FSSGameplayAttributeDataClampedDetailsBase interface
	virtual void InitializeFromStructHandle(const TSharedRef<IPropertyHandle>& InStructPropertyHandle, IPropertyTypeCustomizationUtils& InStructCustomizationUtils) override;
	virtual FGameplayAttributeData* GetGameplayAttributeData() const override;
	//~ End FSSGameplayAttributeDataClampedDetailsBase interface

	//~ Begin IPropertyTypeCustomization interface
	virtual void CustomizeHeader(TSharedRef<IPropertyHandle> InStructPropertyHandle, FDetailWidgetRow& InHeaderRow, IPropertyTypeCustomizationUtils& InStructCustomizationUtils) override;
	virtual void CustomizeChildren(TSharedRef<IPropertyHandle> InStructPropertyHandle, IDetailChildrenBuilder& InStructBuilder, IPropertyTypeCustomizationUtils& InStructCustomizationUtils) override;
	//~ End IPropertyTypeCustomization interface

	FSagaClampedAttributeData* GetGameplayClampedAttributeData() const;

private:
	/** Keep track of the FSagaClampedAttributeData struct we are editing */
	TSharedPtr<FSagaClampedAttributeData*> AttributeDataBeingCustomized;

	TSharedPtr<FSSGameplayAttributeDataDetailsRow> BaseValueRow;

	void HandleBaseValueChanged(float InValue) const;
};
