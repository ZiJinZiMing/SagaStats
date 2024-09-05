// Copyright 2022-2024 Mickael Daniel. All Rights Reserved.

#include "SSNewAttributeViewModel.h"

FSSNewAttributeViewModel::FSSNewAttributeViewModel()
	: bIsReplicated(true)
{
}

void FSSNewAttributeViewModel::Initialize()
{
}

FString FSSNewAttributeViewModel::ToString() const
{
	return FString::Printf(
		TEXT("VariableName: %s, Description: %s, bIsReplicated: %s, CustomizedBlueprint: %s, PinType: %s"),
		*GetVariableName(),
		*GetDescription(),
		GetbIsReplicated() ? TEXT("true") : TEXT("false"),
		*GetNameSafe(GetCustomizedBlueprint().Get()),
		*GetNameSafe(GetPinType().PinSubCategoryObject.Get())
	);
}
