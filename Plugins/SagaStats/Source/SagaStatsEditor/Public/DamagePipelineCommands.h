// DamagePipelineCommands.h — DamagePipeline 编辑器命令集（Build + F7）
#pragma once

#include "CoreMinimal.h"
#include "Framework/Commands/Commands.h"
#include "Styling/AppStyle.h"

class FDamagePipelineCommands : public TCommands<FDamagePipelineCommands>
{
public:
	FDamagePipelineCommands()
		: TCommands<FDamagePipelineCommands>(
			TEXT("DamagePipelineEditor"),
			NSLOCTEXT("Contexts", "DamagePipelineEditor", "Damage Pipeline Editor"),
			NAME_None,
			FAppStyle::GetAppStyleSetName())
	{
	}

	virtual void RegisterCommands() override;

	TSharedPtr<FUICommandInfo> Build;
};
