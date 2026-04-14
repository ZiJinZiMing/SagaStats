// Copyright 2026 StraySpark. All Rights Reserved.

using UnrealBuildTool;

public class UltraWireRenderer : ModuleRules
{
	public UltraWireRenderer(ReadOnlyTargetRules Target) : base(Target)
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
			"MaterialEditor",
			"EditorStyle",
			"InputCore",
			"KismetWidgets"
		});
	}
}
