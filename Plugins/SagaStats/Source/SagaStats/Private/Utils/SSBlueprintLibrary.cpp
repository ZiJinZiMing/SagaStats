// Copyright 2022-2024 Mickael Daniel. All Rights Reserved.

#include "Utils/SSBlueprintLibrary.h"

#include "SSAttributeSetBlueprintBase.h"

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
