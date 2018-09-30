// Copyright(c) 2018 PaperSloth

using UnrealBuildTool;
using System.Collections.Generic;

public class GoTemplateTarget : TargetRules
{
    public GoTemplateTarget(TargetInfo Target) : base(Target)
    {
        Type = TargetType.Game;

        ExtraModuleNames.AddRange(new string[] { "GoTemplate" });
    }
}
