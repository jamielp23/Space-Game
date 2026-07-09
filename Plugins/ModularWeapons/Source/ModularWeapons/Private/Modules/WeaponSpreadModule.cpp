// Copyright Continuum Saga. All Rights Reserved.

#include "Modules/WeaponSpreadModule.h"

#include "Core/WeaponInstance.h"
#include "Core/WeaponTypes.h"

float UWeaponSpreadModule::GetCurrentSpreadDegrees(const UWeaponInstance* Instance) const
{
	if (!Instance)
	{
		return BaseSpreadDegrees;
	}

	const float RestSpread = Instance->bIsAiming ? AimSpreadDegrees : BaseSpreadDegrees;
	return RestSpread + Instance->CurrentBloomDegrees;
}

FVector UWeaponSpreadModule::GetSpreadDirection(const FWeaponFireContext& Context, const FVector& BaseDir) const
{
	const float HalfAngleDeg = GetCurrentSpreadDegrees(Context.Instance);
	if (HalfAngleDeg <= KINDA_SMALL_NUMBER)
	{
		return BaseDir.GetSafeNormal();
	}

	const float HalfAngleRad = FMath::DegreesToRadians(HalfAngleDeg);
	return FMath::VRandCone(BaseDir, HalfAngleRad).GetSafeNormal();
}

void UWeaponSpreadModule::OnShotFired(UWeaponInstance* Instance) const
{
	if (!Instance)
	{
		return;
	}
	Instance->CurrentBloomDegrees = FMath::Min(Instance->CurrentBloomDegrees + SpreadPerShot, MaxBloomDegrees);
}

void UWeaponSpreadModule::TickRecovery(UWeaponInstance* Instance, float DeltaSeconds) const
{
	if (!Instance || Instance->CurrentBloomDegrees <= 0.0f)
	{
		return;
	}
	Instance->CurrentBloomDegrees = FMath::Max(0.0f, Instance->CurrentBloomDegrees - BloomRecoveryPerSecond * DeltaSeconds);
}
