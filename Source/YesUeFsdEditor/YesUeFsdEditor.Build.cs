// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class YesUeFsdEditor : ModuleRules
{
	public YesUeFsdEditor(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core",
			"CoreUObject",
			"Engine",
			"YesUeFsd"
		});

		PrivateDependencyModuleNames.AddRange(new string[]
		{
			// Core Editor
			"UnrealEd",
			"EditorSubsystem",

			// HTTP Server
			"HTTP",
			"HTTPServer",

			// JSON
			"Json",
			"JsonUtilities",

			// UI/Editor
			"Slate",
			"SlateCore",
			"ToolMenus",
			"EditorStyle",

			// Level Editing
			"LevelEditor",
			"EditorScriptingUtilities",

			// Input
			"InputCore",
			"EnhancedInput",

			// AI & Navigation
			"AIModule",
			"NavigationSystem",

			// Play-In-Editor
			"UnrealEd",

			// Python Scripting (Optional)
			"PythonScriptPlugin"
		});
	}
}
