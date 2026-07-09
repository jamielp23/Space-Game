// Copyright Continuum Saga. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Modules/Firing/WeaponFiringModule.h"
#include "Projectiles/WeaponProjectile.h"
#include "ProjectileFiringModule.generated.h"

/**
 * Spawns travelling projectile actors (bolts, rockets, grenade-launcher rounds).
 *
 * Damage, explosion radius and fuse are configured here and pushed into each
 * spawned AWeaponProjectile, so the same projectile class serves everything
 * from a fast energy bolt to a slow arcing rocket.
 */
UCLASS(meta = (DisplayName = "Projectile Firing"))
class MODULARWEAPONS_API UProjectileFiringModule : public UWeaponFiringModule
{
	GENERATED_BODY()

public:
	/** Projectile actor class to spawn (Blueprint subclass encouraged for visuals). */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Projectile")
	TSubclassOf<AWeaponProjectile> ProjectileClass;

	/** Muzzle speed of the projectile in cm/s. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Projectile", meta = (ClampMin = "0.0"))
	float MuzzleSpeed = 6000.0f;

	/** Gravity scale applied to the projectile (0 = straight line, 1 = full gravity). */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Projectile", meta = (ClampMin = "0.0"))
	float GravityScale = 0.0f;

	/** Spawn-time parameters (damage / explosion / fuse) copied into each projectile. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Projectile")
	FWeaponProjectileParams ProjectileParams;

	//~ Begin UWeaponFiringModule
	virtual void ExecuteFire(const FWeaponFireContext& Context) override;
	//~ End UWeaponFiringModule
};
