using UnrealBuildTool;
public class ArriettyRowTarget : TargetRules {
    public ArriettyRowTarget(TargetInfo Target) : base(Target) {
        Type=TargetType.Game;
        DefaultBuildSettings=BuildSettingsVersion.V7;
        IncludeOrderVersion=EngineIncludeOrderVersion.Unreal5_8;
        ExtraModuleNames.Add("ArriettyRow");
    }
}
