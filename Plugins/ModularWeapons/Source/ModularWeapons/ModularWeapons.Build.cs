// Copyright Continuum Saga. All Rights Reserved.

using UnrealBuildTool;

/**
 * Build rules for the ModularWeapons runtime module.
 *
 * The module is intentionally light on dependencies so it can be dropped into
 * any UE5.6 project. GameplayTags is used for data-driven weapon/ammo
 * identification and Niagara for muzzle / impact / tracer effects.
 */
public class ModularWeapons : ModuleRules
{
	public ModularWeapons(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core",
			"CoreUObject",
			"Engine",
			"GameplayTags",
			"Niagara"
		});

		PrivateDependencyModuleNames.AddRange(new string[]
		{
			"PhysicsCore"
		});
	}
}
