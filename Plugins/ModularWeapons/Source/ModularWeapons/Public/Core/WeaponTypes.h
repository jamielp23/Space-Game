// Copyright Continuum Saga. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "WeaponTypes.generated.h"

class UWeaponInstance;
class UWeaponDefinition;
class UWeaponComponent;
class UPrimitiveComponent;
class UWorld;
class APawn;
class AController;

/**
 * High level classification of a weapon. Purely descriptive data used for UI,
 * inventory rules and animation selection. It intentionally does NOT drive
 * firing behaviour: behaviour comes from the composed modules on the
 * UWeaponDefinition. Two "Pistol" weapons can behave completely differently.
 */
UENUM(BlueprintType)
enum class EWeaponType : uint8
{
	Pistol			UMETA(DisplayName = "Pistol"),
	AssaultRifle	UMETA(DisplayName = "Assault Rifle"),
	Shotgun			UMETA(DisplayName = "Shotgun"),
	Sniper			UMETA(DisplayName = "Sniper"),
	Energy			UMETA(DisplayName = "Energy"),
	Heavy			UMETA(DisplayName = "Heavy"),
	Throwable		UMETA(DisplayName = "Throwable")
};

/** How the trigger translates trigger-down / trigger-held into shots. */
UENUM(BlueprintType)
enum class EWeaponFireMode : uint8
{
	/** One shot per trigger pull (semi-automatic pistols, snipers). */
	SingleShot	UMETA(DisplayName = "Single Shot"),
	/** A fixed number of shots per trigger pull, on cadence. */
	Burst		UMETA(DisplayName = "Burst"),
	/** Continuous fire while the trigger is held (rifles, energy). */
	FullAuto	UMETA(DisplayName = "Full Auto")
};

/** Runtime lifecycle state of a single weapon instance. */
UENUM(BlueprintType)
enum class EWeaponState : uint8
{
	Idle		UMETA(DisplayName = "Idle"),
	Firing		UMETA(DisplayName = "Firing"),
	Reloading	UMETA(DisplayName = "Reloading"),
	Overheated	UMETA(DisplayName = "Overheated"),
	Equipping	UMETA(DisplayName = "Equipping")
};

/**
 * The resource model a weapon uses to gate firing. Selected on the Ammo module
 * so a single, independent system covers every weapon class:
 *  - Magazine : conventional mag + reserve + reload (pistols, rifles, shotguns).
 *  - Heat     : builds heat per shot, cools over time, locks out on overheat (energy).
 *  - Charges  : a small finite pool, no reload (throwables, some heavy weapons).
 *  - Infinite : never runs out (debug / sidearms / mounted).
 */
UENUM(BlueprintType)
enum class EAmmoModel : uint8
{
	Magazine	UMETA(DisplayName = "Magazine"),
	Heat		UMETA(DisplayName = "Heat"),
	Charges		UMETA(DisplayName = "Charges"),
	Infinite	UMETA(DisplayName = "Infinite")
};

/**
 * Transient, stack-only context handed to a firing module for a single shot.
 *
 * This is a plain C++ struct (not a USTRUCT) on purpose: it lives only for the
 * duration of a single ExecuteFire call and holds borrowed pointers. It is
 * never stored, serialised, or exposed to Blueprint, so it needs no reflection
 * or garbage-collection bookkeeping.
 */
struct MODULARWEAPONS_API FWeaponFireContext
{
	/** The runtime weapon doing the firing (owns mutable state). */
	UWeaponInstance* Instance = nullptr;

	/** The data asset describing the weapon (treated as read-only during firing). */
	UWeaponDefinition* Definition = nullptr;

	/** The component that manages this weapon on the owning actor. */
	UWeaponComponent* OwnerComponent = nullptr;

	/** Pawn that pulled the trigger (may be null for turrets etc). */
	APawn* InstigatorPawn = nullptr;

	/** Controller responsible for the shot (used for damage attribution). */
	AController* InstigatorController = nullptr;

	/** World to spawn traces / actors / effects in. Never null when firing. */
	UWorld* World = nullptr;

	/** Origin used for hitscan traces and aim (typically the camera / eyes). */
	FVector ViewLocation = FVector::ZeroVector;

	/** Physical muzzle location, used for FX and projectile spawning. */
	FVector MuzzleLocation = FVector::ZeroVector;

	/** Normalised direction the shot is aimed. */
	FVector AimDirection = FVector::ForwardVector;

	/** Component whose socket the muzzle is on (optional, for attaching FX). */
	UPrimitiveComponent* MuzzleComponent = nullptr;

	/** Monotonic index of this shot within a sustained trigger hold (for recoil patterns). */
	int32 ShotIndex = 0;
};
