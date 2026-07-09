// Copyright Continuum Saga. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Modules/Firing/WeaponFiringModule.h"
#include "Projectiles/WeaponProjectile.h"
#include "ThrowableFiringModule.generated.h"

/**
 * Throwing strategy for grenades and other thrown ordnance.
 *
 * Spawns an arcing, gravity-affected projectile with an upward launch bias and
 * (typically) a timed fuse. Pair with the Charges ammo model so the weapon has
 * a small finite number of throws and no reload.
 */
UCLASS(meta = (DisplayName = "Throwable Firing"))
class MODULARWEAPONS_API UThrowableFiringModule : public UWeaponFiringModule
{
	GENERATED_BODY()

public:
	/** Thrown ordnance actor class. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Throwable")
	TSubclassOf<AWeaponProjectile> ThrowableClass;

	/** Throw speed in cm/s. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Throwable", meta = (ClampMin = "0.0"))
	float ThrowSpeed = 1600.0f;

	/** Upward bias added to the aim direction (0 = flat, 1 = 45 degrees up). */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Throwable", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float UpwardArc = 0.35f;

	/** Gravity scale for the arc (1 = full gravity). */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Throwable", meta = (ClampMin = "0.0"))
	float GravityScale = 1.0f;

	/** Spawn-time parameters (explosion / fuse) copied into the thrown actor. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Throwable")
	FWeaponProjectileParams ThrowableParams;

	//~ Begin UWeaponFiringModule
	virtual void ExecuteFire(const FWeaponFireContext& Context) override;
	//~ End UWeaponFiringModule
};
