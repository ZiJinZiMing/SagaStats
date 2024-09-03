using System.IO;
using UnrealBuildTool;

public class SagaStatsEditor : ModuleRules
{
	public SagaStatsEditor(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
		ShortName = "SagaStatsEditor";

		/*
		var EngineDir = Path.GetFullPath(Target.RelativeEnginePath);

		// NOTE: General rule is not to access the private folder of another module,
		// but to use SPathPicker (used in SGBANewDataTableWindowContent), we need to include some private headers
		PrivateIncludePaths.AddRange(
			new string[]
			{
				// For access to SPathPicker (used in SGBANewDataTableWindowContent)
				Path.Combine(EngineDir, @"Source/Editor/ContentBrowser/Private")
			}
		);
		*/
		
		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				"AssetTools",
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
				"Projects",
				"Slate",
				"SlateCore",
				"ToolWidgets",
				"UnrealEd",
			}
		);
	}
}