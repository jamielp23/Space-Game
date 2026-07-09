// Copyright Continuum Saga. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Modules/WeaponModule.h"
#include "Curves/CurveFloat.h"
#include "WeaponRecoilModule.generated.h"

/**
 * Independent recoil system.
 *
 * Produces a per-shot view kick as a (pitch, yaw) impulse in degrees. It only
 * computes the impulse; how that impulse is applied (instant camera punch,
 * spring recovery, animation) is the responsibility of the consumer that binds
 * to UWeaponInstance::OnRecoil, keeping the weapon decoupled from the camera.
 */
UCLASS(meta = (DisplayName = "Recoil"))
class MODULARWEAPONS_API UWeaponRecoilModule : public UWeaponModule
{
	GENERATED_BODY()

public:
	/** Upward pitch kick per shot, in degrees. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Recoil", meta = (ClampMin = "0.0"))
	float VerticalKickDegrees = 0.4f;

	/** Maximum absolute horizontal (yaw) kick per shot, in degrees. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Recoil", meta = (ClampMin = "0.0"))
	float HorizontalKickDegrees = 0.15f;

	/**
	 * Bias applied to horizontal kick in the range [-1, 1]. 0 = symmetric,
	 * positive drifts right, negative drifts left. Lets designers author a
	 * signature recoil pull for a weapon.
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Recoil", meta = (ClampMin = "-1.0", ClampMax = "1.0"))
	float HorizontalBias = 0.0f;

	/**
	 * Multiplies vertical kick as sustained fire continues, indexed by shot.
	 * Empty curve = constant kick. Lets designers make the climb ramp up.
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Recoil")
	FRuntimeFloatCurve VerticalClimbOverShots;

	/**
	 * Recommended recovery speed (interp alpha per second) the consumer can use
	 * to return the view after the kick. Stored here so recoil feel is fully
	 * data-driven even though this module does not apply the recovery itself.
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Recoil", meta = (ClampMin = "0.0"))
	float RecoverySpeed = 10.0f;

	/**
	 * Compute the recoil impulse for a shot.
	 * @param ShotIndex monotonic shot index within the current trigger hold.
	 * @return X = pitch (up positive), Y = yaw (right positive), in degrees.
	 */
	UFUNCTION(BlueprintCallable, Category = "Recoil")
	FVector2D GetRecoilImpulse(int32 ShotIndex) const;
};
