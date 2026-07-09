// Copyright Continuum Saga. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Modules/Firing/WeaponFiringModule.h"
#include "Engine/EngineTypes.h"
#include "BeamFiringModule.generated.h"

class UNiagaraSystem;

/**
 * Energy-weapon firing strategy.
 *
 * Behaves like a high-cadence hitscan but renders a persistent beam from muzzle
 * to impact for each shot. Pairing this with a high rounds-per-minute and the
 * Heat ammo model on the weapon definition produces a continuous energy beam
 * that overheats under sustained use.
 */
UCLASS(meta = (DisplayName = "Beam Firing"))
class MODULARWEAPONS_API UBeamFiringModule : public UWeaponFiringModule
{
	GENERATED_BODY()

public:
	/** Maximum beam length in cm. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Beam", meta = (ClampMin = "1.0"))
	float MaxRange = 12000.0f;

	/** Collision channel used for the beam trace. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Beam")
	TEnumAsByte<ECollisionChannel> TraceChannel = ECC_Visibility;

	/** Beam Niagara system; its "BeamEnd" user vector is set to the impact point. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Beam|FX")
	TSoftObjectPtr<UNiagaraSystem> BeamFX;

	/** Impact Niagara system spawned where the beam lands. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Beam|FX")
	TSoftObjectPtr<UNiagaraSystem> ImpactFX;

	//~ Begin UWeaponFiringModule
	virtual void ExecuteFire(const FWeaponFireContext& Context) override;
	//~ End UWeaponFiringModule
};
