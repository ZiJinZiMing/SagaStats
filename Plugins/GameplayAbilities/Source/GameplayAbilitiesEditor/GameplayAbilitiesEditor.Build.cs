// Copyright Epic Games, Inc. All Rights Reserved.

namespace UnrealBuildTool.Rules
{
	public class GameplayAbilitiesEditor : ModuleRules
	{
		public GameplayAbilitiesEditor(ReadOnlyTargetRules Target) : base(Target)
		{
			PublicDependencyModuleNames.Add("GameplayTasks");

			// GameplayTagsEditor 的 SAddNewGameplayTagWidget.h 在 Internal/ 目录下，
			// 从引擎移到项目插件后需要显式加 include 路径
			PrivateIncludePathModuleNames.Add("GameplayTagsEditor");
			string EngineDir = System.IO.Path.GetFullPath(Target.RelativeEnginePath);
			PrivateIncludePaths.Add(
				System.IO.Path.Combine(EngineDir, "Plugins/Editor/GameplayTagsEditor/Source/GameplayTagsEditor/Internal")
			);

			PrivateDependencyModuleNames.AddRange(
				new string[]
				{
					// ... add private dependencies that you statically link with here ...
					"Core",
					"CoreUObject",
					"Engine",
					"AssetTools",
					"ClassViewer",
					"GameplayTags",
					"GameplayTagsEditor",
					"GameplayAbilities",
					"GameplayTasksEditor",
					"InputCore",
					"PropertyEditor",
					"Slate",
					"SlateCore",					
					"BlueprintGraph",
					"Kismet",
					"KismetCompiler",
					"GraphEditor",
					"LevelSequence",
					"MainFrame",
					"EditorFramework",
					"UnrealEd",
					"WorkspaceMenuStructure",
					"ContentBrowser",
					"EditorWidgets",
					"SourceControl",
					"SequencerCore",
					"Sequencer",
					"MovieSceneTools",
					"MovieScene",
					"DataRegistry",
					"DataRegistryEditor",
					"ToolMenus",
					"ApplicationCore"
				}
			);
		}
	}
}
