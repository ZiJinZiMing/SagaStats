// Copyright 2026 StraySpark. All Rights Reserved.

using UnrealBuildTool;

public class UltraWireSettings : ModuleRules
{
	public UltraWireSettings(ReadOnlyTargetRules Target) : base(Target)
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
			"EditorStyle",
			"InputCore",
			"PropertyEditor",
			"DesktopPlatform"
		});
	}
}
