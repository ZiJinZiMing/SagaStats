/***************************************************************************************************************
* Plugin:       SagaStats
* Author:       Claude(@JinmingZhang)
* Description:  SagaStats offers modular damage process and meter systems to support adaptable status management
****************************************************************************************************************/

// DamagePipelineCommands.cpp — 命令注册（Build chord = F7）
#include "DamagePipelineCommands.h"
#include "Framework/Commands/UICommandInfo.h"
#include "InputCoreTypes.h"

#define LOCTEXT_NAMESPACE "DamagePipelineCommands"

void FDamagePipelineCommands::RegisterCommands()
{
	UI_COMMAND(Build, "Build", "Build pipeline (topo sort + refresh graph)",
		EUserInterfaceActionType::Button, FInputChord(EKeys::F7));
}

#undef LOCTEXT_NAMESPACE
