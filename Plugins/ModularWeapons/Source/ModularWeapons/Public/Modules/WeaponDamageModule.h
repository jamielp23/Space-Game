// Copyright Continuum Saga. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Modules/WeaponModule.h"
#include "Curves/CurveFloat.h"
#include "WeaponDamageModule.generated.h"

struct FWeaponFireContext;

/**
 * Independent damage system.
 *
 * Owns everything about "how much does a hit hurt": base damage, distance
 * falloff, headshot / weakpoint multipliers and the UDamageType used for
 * attribution. It applies damage through the engine's standard damage pipeline
 * (ApplyPointDamage / ApplyRadialDamage) so it stays fully decoupled from any
 * particular health or attribute system.
 */
UCLASS(meta = (DisplayName = "Damage"))
class MODULARWEAPONS_API UWeaponDamageModule : public UWeaponModule
{
	GENERATED_BODY()

public:
	/** Base damage of a single projectile / trace before any multipliers. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Damage", meta = (ClampMin = "0.0"))
	float BaseDamage = 20.0f;

	/** Damage type used for attribution and resistance handling. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Damage")
	TSubclassOf<UDamageType> DamageType;

	/**
	 * Multiplier applied to BaseDamage as a function of distance (in cm).
	 * Leave empty for no falloff (multiplier of 1 at any range). Snipers might
	 * stay flat; shotguns fall off sharply past a few metres.
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Damage")
	FRuntimeFloatCurve DamageFalloffOverDistance;

	/** Multiplier applied when the hit bone matches a critical bone. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Damage", meta = (ClampMin = "1.0"))
	float CriticalHitMultiplier = 2.0f;

	/** Bones that count as critical / weak points (e.g. "head", "neck"). */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Damage")
	TArray<FName> CriticalHitBones;

	/**
	 * Compute the final damage for a hit at a given distance, accounting for
	 * falloff and critical-bone multipliers.
	 */
	UFUNCTION(BlueprintCallable, Category = "Damage")
	float ComputeDamage(float Distance, const FHitResult& Hit) const;

	/**
	 * Apply point damage for a single hit through the engine damage pipeline.
	 * @return the amount of damage that was applied.
	 */
	float ApplyPointDamage(const FWeaponFireContext& Context, const FHitResult& Hit) const;

	/** True if the hit bone is configured as a critical / weak point. */
	bool IsCriticalHit(const FHitResult& Hit) const;
};
