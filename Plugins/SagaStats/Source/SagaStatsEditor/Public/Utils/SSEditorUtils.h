﻿/******************************************************************************************
* Plugin:       SagaStats
* Author:       Jinming Zhang
* Description:  SagaStats is a status system that supports fully blueprintable attribute definitions and value calculations.
******************************************************************************************/
#pragma once

#include "CoreMinimal.h"

class FSSBlueprintEditor;
class IDetailLayoutBuilder;
class UAttributeSet;
class UBlueprint;

namespace UE::SS::EditorUtils
{
	UAttributeSet* GetAttributeBeingCustomized(const IDetailLayoutBuilder& InDetailLayout);
	UBlueprint* GetBlueprintFromClass(const UClass* InClass);
	TWeakPtr<FSSBlueprintEditor> FindBlueprintEditorForAsset(UObject* InObject);
}
