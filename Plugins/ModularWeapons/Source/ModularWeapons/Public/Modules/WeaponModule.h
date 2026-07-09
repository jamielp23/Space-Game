// Copyright Continuum Saga. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "WeaponModule.generated.h"

/**
 * Base class for every composable weapon "system".
 *
 * A weapon is assembled from independent modules (firing, ammo, spread, recoil,
 * damage). Each module is an EditInline, instanced UObject stored on a
 * UWeaponDefinition, which means designers configure them directly inside the
 * weapon Data Asset with no code and no separate assets.
 *
 * Modules are pure configuration + behaviour. They hold NO mutable per-shot
 * state, because a single UWeaponDefinition (and therefore its module graph) is
 * shared by every UWeaponInstance created from it. All runtime state lives on
 * the UWeaponInstance and is passed to module functions explicitly. This keeps
 * every system independent and trivially reusable across weapon types.
 */
UCLASS(Abstract, EditInlineNew, DefaultToInstanced, BlueprintType, Blueprintable, CollapseCategories)
class MODULARWEAPONS_API UWeaponModule : public UObject
{
	GENERATED_BODY()

public:
	/** Optional designer label to make a module easy to identify in the details panel. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Module")
	FName ModuleName;
};
