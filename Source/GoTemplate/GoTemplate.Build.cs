// Copyright(c) 2018 PaperSloth

using UnrealBuildTool;

public class GoTemplate : ModuleRules
{
    public GoTemplate(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore" });

        PrivateDependencyModuleNames.AddRange(new string[] { "HeadMountedDisplay", "NavigationSystem" });
    }
}
