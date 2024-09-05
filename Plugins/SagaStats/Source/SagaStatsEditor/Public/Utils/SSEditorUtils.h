// Copyright 2022-2024 Mickael Daniel. All Rights Reserved.

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
