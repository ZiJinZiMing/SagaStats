using UnrealBuildTool;

public class SagaStatsEditor : ModuleRules
{
	public SagaStatsEditor(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
		ShortName = "SSEditor";

		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				"SagaStats",
			}
		);

		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"CoreUObject",
				"Engine",
				"BlueprintGraph",
				"DeveloperSettings",
				"EditorStyle",
				"EditorSubsystem",
				"EditorFramework",
				"GameplayAbilities",
				"GameplayAbilitiesEditor",
				"GameplayTags",
				"GraphEditor",
				"InputCore",
				"Kismet",
				"KismetWidgets",
				"MainFrame", // 5.0 only ?
				"MessageLog",
				"RigVMDeveloper",
				"Slate",
				"SlateCore",
				"ToolWidgets",
				"UnrealEd",
			}
		);
	}
}