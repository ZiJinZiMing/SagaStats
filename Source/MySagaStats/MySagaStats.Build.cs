// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class MySagaStats : ModuleRules
{
	public MySagaStats(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] {
			"Core",
			"CoreUObject",
			"Engine",
			"InputCore",
			"EnhancedInput",
			"AIModule",
			"StateTreeModule",
			"GameplayStateTreeModule",
			"UMG",
			"Slate"
		});

		PrivateDependencyModuleNames.AddRange(new string[] { });

		PublicIncludePaths.AddRange(new string[] {
			"MySagaStats",
			"MySagaStats/Variant_Platforming",
			"MySagaStats/Variant_Platforming/Animation",
			"MySagaStats/Variant_Combat",
			"MySagaStats/Variant_Combat/AI",
			"MySagaStats/Variant_Combat/Animation",
			"MySagaStats/Variant_Combat/Gameplay",
			"MySagaStats/Variant_Combat/Interfaces",
			"MySagaStats/Variant_Combat/UI",
			"MySagaStats/Variant_SideScrolling",
			"MySagaStats/Variant_SideScrolling/AI",
			"MySagaStats/Variant_SideScrolling/Gameplay",
			"MySagaStats/Variant_SideScrolling/Interfaces",
			"MySagaStats/Variant_SideScrolling/UI"
		});

		// Uncomment if you are using Slate UI
		// PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });

		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
	}
}
