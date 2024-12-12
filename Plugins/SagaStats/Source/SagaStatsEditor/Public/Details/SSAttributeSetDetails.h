/******************************************************************************************
* Plugin:       SagaStats
* Author:       Jinming Zhang
* Description:  SagaStats is a status system that supports fully blueprintable attribute definitions and value calculations.
******************************************************************************************/
#pragma once

#include "CoreMinimal.h"
#include "IDetailCustomization.h"
#include "UObject/WeakObjectPtr.h"

class FSSBlueprintEditor;
class FSSNewAttributeViewModel;
class SSSPositiveActionButton;
class SWidget;
class UAttributeSet;
class UBlueprint;

/**
 * Details customization for Attribute Sets
 *
 * Mainly to add a "+" button to add a new attribute member variable.
 */
class FSSAttributeSetDetails : public IDetailCustomization
{
public:
	FSSAttributeSetDetails() = default;

	/** Makes a new instance of this detail layout class for a specific detail view requesting it */
	static TSharedRef<IDetailCustomization> MakeInstance();

	virtual void CustomizeDetails(IDetailLayoutBuilder& InDetailLayout) override;

private:
	/** The UObject we are editing */
	TWeakObjectPtr<UAttributeSet> AttributeBeingCustomized;

	/** The blueprint we are editing */
	TWeakObjectPtr<UBlueprint> BlueprintBeingCustomized;
	
	/** Weak reference to the Blueprint editor */
	TWeakPtr<FSSBlueprintEditor> BlueprintEditorPtr;

	/** Constructs a DetailsView widget for the new attribute window */
	TSharedRef<SWidget> CreateNewAttributeVariableWidget();

	static void HandleAttributeWindowCancel(const TSharedPtr<FSSNewAttributeViewModel>& InViewModel);
	void HandleAttributeWindowFinish(const TSharedPtr<FSSNewAttributeViewModel>& InViewModel) const;
};
