// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class YesUeFsd : ModuleRules
{
	public YesUeFsd(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core",
			"CoreUObject",
			"Engine",
			"InputCore",
			"EnhancedInput",
			"UMG",
			"Slate",
			"SlateCore"
		});

		PrivateDependencyModuleNames.AddRange(new string[]
		{
			"Json",
			"JsonUtilities",
			"AIModule",
			"NavigationSystem"
		});
	}
}
