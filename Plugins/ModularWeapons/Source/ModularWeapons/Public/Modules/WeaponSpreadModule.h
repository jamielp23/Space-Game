// Copyright Continuum Saga. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Modules/WeaponModule.h"
#include "WeaponSpreadModule.generated.h"

struct FWeaponFireContext;
class UWeaponInstance;

/**
 * Independent accuracy / spread system.
 *
 * Turns a perfect aim direction into a realistic cone, and models dynamic
 * "bloom" that grows as the weapon is fired and recovers when it is not. The
 * same module covers a laser-accurate sniper (tiny cone, no bloom) and a
 * hip-fired shotgun (wide fixed cone) purely through data.
 *
 * Per-shot bloom state lives on the UWeaponInstance; this module only reads /
 * writes it through the passed-in instance, holding no state itself.
 */
UCLASS(meta = (DisplayName = "Spread"))
class MODULARWEAPONS_API UWeaponSpreadModule : public UWeaponModule
{
	GENERATED_BODY()

public:
	/** Half-angle of the spread cone (degrees) when hip-firing at rest. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Spread", meta = (ClampMin = "0.0"))
	float BaseSpreadDegrees = 1.5f;

	/** Half-angle of the spread cone (degrees) when aiming down sights at rest. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Spread", meta = (ClampMin = "0.0"))
	float AimSpreadDegrees = 0.25f;

	/** Additional spread (degrees) added to the current bloom on every shot. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Spread|Bloom", meta = (ClampMin = "0.0"))
	float SpreadPerShot = 0.6f;

	/** Maximum bloom (degrees) that can accumulate from sustained fire. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Spread|Bloom", meta = (ClampMin = "0.0"))
	float MaxBloomDegrees = 5.0f;

	/** How fast accumulated bloom recovers (degrees per second) when not firing. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Spread|Bloom", meta = (ClampMin = "0.0"))
	float BloomRecoveryPerSecond = 6.0f;

	/** Total effective spread half-angle right now for the given instance. */
	UFUNCTION(BlueprintCallable, Category = "Spread")
	float GetCurrentSpreadDegrees(const UWeaponInstance* Instance) const;

	/** Return a randomised direction inside the current spread cone. */
	FVector GetSpreadDirection(const FWeaponFireContext& Context, const FVector& BaseDir) const;

	/** Called once per shot: accumulate bloom on the instance. */
	void OnShotFired(UWeaponInstance* Instance) const;

	/** Called every tick: recover accumulated bloom over time. */
	void TickRecovery(UWeaponInstance* Instance, float DeltaSeconds) const;
};
