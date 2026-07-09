// Copyright Continuum Saga. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Modules/WeaponModule.h"
#include "Core/WeaponTypes.h"
#include "WeaponAmmoModule.generated.h"

class UWeaponInstance;

/**
 * Independent resource / ammunition system.
 *
 * A single module supports every weapon class through EAmmoModel:
 *  - Magazine : mag + reserve + reload (optionally shell-by-shell for shotguns).
 *  - Heat     : builds heat per shot, cools over time, hard overheat lockout (energy).
 *  - Charges  : small finite pool, no reload (throwables, rockets).
 *  - Infinite : never gates firing.
 *
 * All mutable counters (current mag, reserve, heat, charges) live on the
 * UWeaponInstance. This module holds only configuration and the logic that
 * reads / mutates the instance's counters.
 */
UCLASS(meta = (DisplayName = "Ammo"))
class MODULARWEAPONS_API UWeaponAmmoModule : public UWeaponModule
{
	GENERATED_BODY()

public:
	/** Which resource model governs this weapon. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ammo")
	EAmmoModel Model = EAmmoModel::Magazine;

	// --- Magazine model --------------------------------------------------

	/** Rounds per magazine. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ammo|Magazine", meta = (ClampMin = "1", EditCondition = "Model == EAmmoModel::Magazine", EditConditionHides))
	int32 MagazineSize = 30;

	/** Rounds carried in reserve at spawn. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ammo|Magazine", meta = (ClampMin = "0", EditCondition = "Model == EAmmoModel::Magazine", EditConditionHides))
	int32 StartingReserve = 120;

	/** Maximum reserve that can be carried. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ammo|Magazine", meta = (ClampMin = "0", EditCondition = "Model == EAmmoModel::Magazine", EditConditionHides))
	int32 MaxReserve = 240;

	/** Seconds a full reload takes. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ammo|Magazine", meta = (ClampMin = "0.0", EditCondition = "Model == EAmmoModel::Magazine", EditConditionHides))
	float ReloadDuration = 2.0f;

	/**
	 * If true the weapon reloads one round at a time (shotgun shells); each
	 * ReloadDuration inserts a single round and the reload can be interrupted.
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ammo|Magazine", meta = (EditCondition = "Model == EAmmoModel::Magazine", EditConditionHides))
	bool bReloadPerRound = false;

	// --- Heat model ------------------------------------------------------

	/** Heat added per shot. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ammo|Heat", meta = (ClampMin = "0.0", EditCondition = "Model == EAmmoModel::Heat", EditConditionHides))
	float HeatPerShot = 8.0f;

	/** Heat at which the weapon overheats and locks out. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ammo|Heat", meta = (ClampMin = "1.0", EditCondition = "Model == EAmmoModel::Heat", EditConditionHides))
	float MaxHeat = 100.0f;

	/** Heat dissipated per second while not overheated. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ammo|Heat", meta = (ClampMin = "0.0", EditCondition = "Model == EAmmoModel::Heat", EditConditionHides))
	float CoolingRate = 30.0f;

	/** Forced lockout duration (seconds) after a full overheat. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ammo|Heat", meta = (ClampMin = "0.0", EditCondition = "Model == EAmmoModel::Heat", EditConditionHides))
	float OverheatLockoutSeconds = 2.5f;

	// --- Charges model ---------------------------------------------------

	/** Maximum number of charges (grenades / rockets). */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ammo|Charges", meta = (ClampMin = "1", EditCondition = "Model == EAmmoModel::Charges", EditConditionHides))
	int32 MaxCharges = 3;

	// --- Behaviour -------------------------------------------------------

	/** Initialise the instance's counters from this configuration. */
	void InitializeState(UWeaponInstance* Instance) const;

	/** Can the weapon fire right now given its resource state? */
	bool CanFire(const UWeaponInstance* Instance) const;

	/** Consume the resource cost of a single shot (mag round / heat / charge). */
	void ConsumeForShot(UWeaponInstance* Instance) const;

	/** True if a (magazine) reload is currently possible and useful. */
	bool CanReload(const UWeaponInstance* Instance) const;

	/** How long a reload takes given the current instance state. */
	float GetReloadDuration(const UWeaponInstance* Instance) const;

	/** Apply the result of a completed reload step to the instance. */
	void CompleteReload(UWeaponInstance* Instance) const;

	/** Advance passive cooling / overheat recovery for the Heat model. */
	void TickCooling(UWeaponInstance* Instance, float DeltaSeconds) const;
};
