// Copyright Continuum Saga. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Modules/WeaponModule.h"
#include "WeaponFiringModule.generated.h"

struct FWeaponFireContext;
class UNiagaraSystem;
class USoundBase;

/**
 * Abstract strategy for turning a single "shot" into world effects.
 *
 * This is the extensibility seam of the whole architecture. Each concrete
 * subclass (hitscan, projectile, beam, throwable) is one self-contained way a
 * shot manifests. Designers pick a firing strategy per weapon in the Data Asset;
 * engineers add new strategies by subclassing this once.
 *
 * A "shot" is one trigger cadence tick and consumes one resource unit, but may
 * emit multiple projectiles/traces (ProjectilesPerShot) — that is how a shotgun
 * fires eight pellets for one shell.
 */
UCLASS(Abstract, EditInlineNew, DefaultToInstanced, BlueprintType)
class MODULARWEAPONS_API UWeaponFiringModule : public UWeaponModule
{
	GENERATED_BODY()

public:
	/** Individual projectiles / traces emitted per shot (shotgun pellets). */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Firing", meta = (ClampMin = "1"))
	int32 ProjectilesPerShot = 1;

	/** Muzzle flash spawned at the muzzle each shot. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Firing|FX")
	TSoftObjectPtr<UNiagaraSystem> MuzzleFX;

	/** Sound played at the muzzle each shot. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Firing|FX")
	TSoftObjectPtr<USoundBase> FireSound;

	/** Socket on the weapon mesh the muzzle FX attaches to. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Firing|FX")
	FName MuzzleSocketName = TEXT("Muzzle");

	/**
	 * Emit a single shot. Implemented by each concrete strategy.
	 * Called after the owning instance has already validated cadence / ammo.
	 */
	virtual void ExecuteFire(const FWeaponFireContext& Context)
		PURE_VIRTUAL(UWeaponFiringModule::ExecuteFire, );

protected:
	/** Shared helper: spawn muzzle flash + fire sound at the muzzle. */
	void PlayFireEffects(const FWeaponFireContext& Context) const;

	/**
	 * Shared helper: run the base aim direction through the weapon's spread
	 * module (if any) to produce a per-projectile direction.
	 */
	FVector ApplySpread(const FWeaponFireContext& Context) const;
};
