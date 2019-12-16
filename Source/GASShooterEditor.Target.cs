// Copyright 2019 Dan Kestranek.

using UnrealBuildTool;
using System.Collections.Generic;

public class GASShooterEditorTarget : TargetRules
{
	public GASShooterEditorTarget( TargetInfo Target) : base(Target)
	{
		Type = TargetType.Editor;

		ExtraModuleNames.AddRange( new string[] { "GASShooter" } );
	}
}
