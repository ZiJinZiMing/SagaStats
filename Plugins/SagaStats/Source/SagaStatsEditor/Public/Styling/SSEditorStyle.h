// Copyright 2022-2024 Mickael Daniel. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Styling/SlateStyle.h"

class SAGASTATSEDITOR_API FSSEditorStyle : public FSlateStyleSet
{
public:
	FSSEditorStyle();
	virtual ~FSSEditorStyle() override;

	static FSSEditorStyle& Get()
	{
		static FSSEditorStyle Inst;
		return Inst;
	}
};
