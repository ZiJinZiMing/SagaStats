// Copyright 2026 StraySpark. All Rights Reserved.

using UnrealBuildTool;

public class UltraWireProfiler : ModuleRules
{
	public UltraWireProfiler(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core",
			"CoreUObject",
			"Engine",
			"Slate",
			"SlateCore",
			"UltraWireCore"
		});

		PrivateDependencyModuleNames.AddRange(new string[]
		{
			"UnrealEd",
			"GraphEditor",
			"BlueprintGraph",
			"Kismet",
			"EditorStyle"
		});
	}
}
