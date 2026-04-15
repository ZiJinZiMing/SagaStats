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
