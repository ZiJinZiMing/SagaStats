// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class MySagaStats : ModuleRules
{
	public MySagaStats(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core", "CoreUObject", "Engine", "InputCore", "EnhancedInput",
		});

		PrivateDependencyModuleNames.AddRange(new string[] { "GameplayAbilities", "GameplayTasks", "GameplayTags", "SagaStats", });

		if (Target.bBuildEditor == true)
		{
			PrivateDependencyModuleNames.Add("SagaStatsEditor");
		}
	}
}