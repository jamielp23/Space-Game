// Copyright Continuum Saga. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "Core/WeaponTypes.h"
#include "WeaponInstance.generated.h"

class UWeaponInstance;
class UWeaponDefinition;
class UWeaponComponent;

/** Fired whenever the magazine or reserve ammo changes. */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnWeaponAmmoChanged, UWeaponInstance*, Weapon, int32, MagazineAmmo, int32, ReserveAmmo);
/** Fired whenever the weapon's lifecycle state changes. */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnWeaponStateChanged, UWeaponInstance*, Weapon, EWeaponState, NewState);
/** Fired for energy weapons whenever heat changes (HeatPercent in [0,1]). */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnWeaponHeatChanged, UWeaponInstance*, Weapon, float, HeatPercent);
/** Fired the instant a shot leaves the weapon. */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnWeaponFired, UWeaponInstance*, Weapon);
/** Fired per shot with the recoil impulse (X=pitch up, Y=yaw right, degrees). */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnWeaponRecoil, UWeaponInstance*, Weapon, FVector2D, RecoilImpulse);

/**
 * A live, per-owner instance of a weapon.
 *
 * The UWeaponDefinition and its module graph are shared, immutable configuration.
 * This object is the mutable half: it holds all runtime counters (ammo, heat,
 * bloom, cadence, reload timers) and orchestrates the modules to actually fire,
 * reload, cool down and recover. The owning UWeaponComponent ticks it.
 *
 * State is public so the independent modules can read / mutate it directly; the
 * counters are also BlueprintReadOnly so UI can bind to them. Mutation always
 * flows through this instance's methods or a module operating on this instance.
 */
UCLASS(BlueprintType)
class MODULARWEAPONS_API UWeaponInstance : public UObject
{
	GENERATED_BODY()

public:
	// --- Setup -----------------------------------------------------------

	/** Bind this instance to a definition and its managing component. */
	void Initialize(UWeaponDefinition* InDefinition, UWeaponComponent* InOwnerComponent);

	/** The immutable definition backing this instance. */
	UFUNCTION(BlueprintPure, Category = "Weapon")
	const UWeaponDefinition* GetDefinition() const { return Definition; }

	/** The component managing this weapon. */
	UFUNCTION(BlueprintPure, Category = "Weapon")
	UWeaponComponent* GetOwnerComponent() const { return OwnerComponent; }

	// --- Trigger control -------------------------------------------------

	/** Trigger pressed. Fires immediately for single/burst; sustains full-auto. */
	UFUNCTION(BlueprintCallable, Category = "Weapon")
	void StartFire();

	/** Trigger released. Ends sustained fire and resets the recoil pattern. */
	UFUNCTION(BlueprintCallable, Category = "Weapon")
	void StopFire();

	/** Begin a reload if the ammo module allows it. */
	UFUNCTION(BlueprintCallable, Category = "Weapon")
	void Reload();

	/** Set aim-down-sights state (tightens spread). */
	UFUNCTION(BlueprintCallable, Category = "Weapon")
	void SetAiming(bool bNewAiming);

	// --- Lifecycle -------------------------------------------------------

	/** Advance cadence, cooling, bloom recovery and reload timers. */
	void Tick(float DeltaSeconds);

	/** Called when this weapon becomes the active weapon. */
	void OnEquipped();

	/** Called when this weapon is holstered. */
	void OnUnequipped();

	// --- Queries ---------------------------------------------------------

	UFUNCTION(BlueprintPure, Category = "Weapon")
	EWeaponState GetState() const { return State; }

	UFUNCTION(BlueprintPure, Category = "Weapon")
	bool IsFiring() const { return bTriggerHeld; }

	/** Heat as a 0..1 fraction of max (0 for non-heat weapons). */
	UFUNCTION(BlueprintPure, Category = "Weapon")
	float GetHeatPercent() const;

	// --- Events ----------------------------------------------------------

	UPROPERTY(BlueprintAssignable, Category = "Weapon|Events")
	FOnWeaponAmmoChanged OnAmmoChanged;

	UPROPERTY(BlueprintAssignable, Category = "Weapon|Events")
	FOnWeaponStateChanged OnStateChanged;

	UPROPERTY(BlueprintAssignable, Category = "Weapon|Events")
	FOnWeaponHeatChanged OnHeatChanged;

	UPROPERTY(BlueprintAssignable, Category = "Weapon|Events")
	FOnWeaponFired OnFired;

	UPROPERTY(BlueprintAssignable, Category = "Weapon|Events")
	FOnWeaponRecoil OnRecoil;

	// --- Runtime state (owned here, mutated by modules) ------------------

	/** Rounds currently in the magazine (Magazine model). */
	UPROPERTY(BlueprintReadOnly, Category = "Weapon|State")
	int32 CurrentAmmoInMag = 0;

	/** Rounds in reserve (Magazine model). */
	UPROPERTY(BlueprintReadOnly, Category = "Weapon|State")
	int32 ReserveAmmo = 0;

	/** Remaining charges (Charges model). */
	UPROPERTY(BlueprintReadOnly, Category = "Weapon|State")
	int32 Charges = 0;

	/** Current heat (Heat model). */
	UPROPERTY(BlueprintReadOnly, Category = "Weapon|State")
	float CurrentHeat = 0.0f;

	/** True while the weapon is locked out from overheating (Heat model). */
	UPROPERTY(BlueprintReadOnly, Category = "Weapon|State")
	bool bOverheated = false;

	/** Remaining forced overheat lockout, seconds. */
	float OverheatTimer = 0.0f;

	/** Accumulated dynamic spread from firing, degrees. */
	UPROPERTY(BlueprintReadOnly, Category = "Weapon|State")
	float CurrentBloomDegrees = 0.0f;

	/** True while aiming down sights. */
	UPROPERTY(BlueprintReadOnly, Category = "Weapon|State")
	bool bIsAiming = false;

private:
	/** Attempt to fire one shot if cadence / state / resources permit. */
	bool TryFireOnce();

	/** Perform one validated shot: emit, consume, recoil, notify. */
	void FireShot();

	/** Transition to a new state and broadcast. */
	void SetState(EWeaponState NewState);

	/** Broadcast ammo/heat change to listeners. */
	void NotifyResourceChanged();

	/** Finish (a step of) a reload. */
	void FinishReload();

	UPROPERTY()
	TObjectPtr<UWeaponDefinition> Definition = nullptr;

	UPROPERTY()
	TObjectPtr<UWeaponComponent> OwnerComponent = nullptr;

	EWeaponState State = EWeaponState::Idle;

	/** Seconds accumulated since the last shot (drives cadence). */
	float TimeSinceLastShot = 1000.0f;

	/** True while the trigger is held. */
	bool bTriggerHeld = false;

	/** Shots remaining in the current burst. */
	int32 BurstShotsRemaining = 0;

	/** True while a burst is in progress (blocks re-trigger mid-burst). */
	bool bBurstActive = false;

	/** Shot index within the current sustained hold (drives recoil pattern). */
	int32 SustainedShotIndex = 0;

	/** Countdown to the next reload step completing. */
	float ReloadTimeRemaining = 0.0f;
};
