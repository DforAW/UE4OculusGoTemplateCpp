// Copyright(c) 2018 PaperSloth

using UnrealBuildTool;
using System.Collections.Generic;

public class GoTemplateEditorTarget : TargetRules
{
    public GoTemplateEditorTarget(TargetInfo Target) : base(Target)
    {
        Type = TargetType.Editor;

        ExtraModuleNames.AddRange(new string[] { "GoTemplate" });
    }
}
