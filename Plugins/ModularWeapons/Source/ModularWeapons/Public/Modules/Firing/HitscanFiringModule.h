// Copyright Continuum Saga. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Modules/Firing/WeaponFiringModule.h"
#include "Engine/EngineTypes.h"
#include "HitscanFiringModule.generated.h"

class UNiagaraSystem;

/**
 * Instant-hit firing strategy: line traces from the view, applies damage at the
 * impact via the weapon's damage module, and spawns impact / tracer FX.
 *
 * Suits pistols, assault rifles, snipers and — with ProjectilesPerShot > 1 and
 * a wide spread module — shotguns.
 */
UCLASS(meta = (DisplayName = "Hitscan Firing"))
class MODULARWEAPONS_API UHitscanFiringModule : public UWeaponFiringModule
{
	GENERATED_BODY()

public:
	/** Maximum trace distance in cm. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Hitscan", meta = (ClampMin = "1.0"))
	float MaxRange = 15000.0f;

	/** Collision channel used for the shot trace. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Hitscan")
	TEnumAsByte<ECollisionChannel> TraceChannel = ECC_Visibility;

	/** Niagara system spawned at each impact point. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Hitscan|FX")
	TSoftObjectPtr<UNiagaraSystem> ImpactFX;

	/** Optional tracer beam spawned from muzzle to impact (uses a "Beam" user vector). */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Hitscan|FX")
	TSoftObjectPtr<UNiagaraSystem> TracerFX;

	//~ Begin UWeaponFiringModule
	virtual void ExecuteFire(const FWeaponFireContext& Context) override;
	//~ End UWeaponFiringModule

private:
	/** Trace and resolve a single pellet / bullet. */
	void FireSingleTrace(const FWeaponFireContext& Context) const;
};
