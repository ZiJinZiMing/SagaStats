/******************************************************************************************
* Plugin:       SagaStats
* Author:       Jinming Zhang
* Description:  SagaStats is a status system that supports fully blueprintable attribute definitions and value calculations.
******************************************************************************************/
#include "Utils/SSBlueprintLibrary.h"

#include "SagaAttributeSet.h"

FString USSBlueprintLibrary::GetDebugStringFromExecutionData(const FSSAttributeSetExecutionData& ExecutionData)
{
	return ExecutionData.ToString(LINE_TERMINATOR);
}

FString USSBlueprintLibrary::GetDebugStringFromExecutionDataSeparator(const FSSAttributeSetExecutionData& ExecutionData, const FString& Separator)
{
	return ExecutionData.ToString(Separator);
}

FString USSBlueprintLibrary::GetDebugStringFromAttribute(const FGameplayAttribute& Attribute)
{
	return Attribute.GetName();
}

bool USSBlueprintLibrary::NotEqual_GameplayAttributeGameplayAttribute(const FGameplayAttribute A, const FString B)
{
	return A.GetName() != B;
}

FText USSBlueprintLibrary::GetAttributeDisplayNameText(const FGameplayAttribute& InAttribute)
{
	return FText::FromString(InAttribute.GetName());
}

bool USSBlueprintLibrary::GetAttributeByPropertyName(const TSubclassOf<UAttributeSet>& AttributeSetClass, const FName& PropertyName, FGameplayAttribute& OutAttribute)
{
	if (FProperty* Property = FindFieldChecked<FProperty>(AttributeSetClass, PropertyName))
	{
		OutAttribute = FGameplayAttribute(Property, AttributeSetClass);
		return true;
	}
	return false;
}
