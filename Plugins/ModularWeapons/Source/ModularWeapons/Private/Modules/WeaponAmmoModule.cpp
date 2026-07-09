// Copyright Continuum Saga. All Rights Reserved.

#include "Modules/WeaponAmmoModule.h"

#include "Core/WeaponInstance.h"

void UWeaponAmmoModule::InitializeState(UWeaponInstance* Instance) const
{
	if (!Instance)
	{
		return;
	}

	switch (Model)
	{
	case EAmmoModel::Magazine:
		Instance->CurrentAmmoInMag = MagazineSize;
		Instance->ReserveAmmo = FMath::Clamp(StartingReserve, 0, MaxReserve);
		break;

	case EAmmoModel::Heat:
		Instance->CurrentHeat = 0.0f;
		Instance->bOverheated = false;
		Instance->OverheatTimer = 0.0f;
		break;

	case EAmmoModel::Charges:
		Instance->Charges = MaxCharges;
		break;

	case EAmmoModel::Infinite:
	default:
		break;
	}
}

bool UWeaponAmmoModule::CanFire(const UWeaponInstance* Instance) const
{
	if (!Instance)
	{
		return false;
	}

	switch (Model)
	{
	case EAmmoModel::Magazine:
		return Instance->CurrentAmmoInMag > 0;

	case EAmmoModel::Heat:
		// Overheated weapons are locked out until the timer expires.
		return !Instance->bOverheated && Instance->CurrentHeat < MaxHeat;

	case EAmmoModel::Charges:
		return Instance->Charges > 0;

	case EAmmoModel::Infinite:
	default:
		return true;
	}
}

void UWeaponAmmoModule::ConsumeForShot(UWeaponInstance* Instance) const
{
	if (!Instance)
	{
		return;
	}

	switch (Model)
	{
	case EAmmoModel::Magazine:
		Instance->CurrentAmmoInMag = FMath::Max(0, Instance->CurrentAmmoInMag - 1);
		break;

	case EAmmoModel::Heat:
		Instance->CurrentHeat += HeatPerShot;
		if (Instance->CurrentHeat >= MaxHeat)
		{
			Instance->CurrentHeat = MaxHeat;
			Instance->bOverheated = true;
			Instance->OverheatTimer = OverheatLockoutSeconds;
		}
		break;

	case EAmmoModel::Charges:
		Instance->Charges = FMath::Max(0, Instance->Charges - 1);
		break;

	case EAmmoModel::Infinite:
	default:
		break;
	}
}

bool UWeaponAmmoModule::CanReload(const UWeaponInstance* Instance) const
{
	if (!Instance || Model != EAmmoModel::Magazine)
	{
		return false;
	}
	// Useful only if there is spare ammo and the magazine is not already full.
	return Instance->ReserveAmmo > 0 && Instance->CurrentAmmoInMag < MagazineSize;
}

float UWeaponAmmoModule::GetReloadDuration(const UWeaponInstance* /*Instance*/) const
{
	return ReloadDuration;
}

void UWeaponAmmoModule::CompleteReload(UWeaponInstance* Instance) const
{
	if (!Instance || Model != EAmmoModel::Magazine)
	{
		return;
	}

	if (bReloadPerRound)
	{
		// Insert a single round; the instance decides whether to keep reloading.
		if (Instance->ReserveAmmo > 0 && Instance->CurrentAmmoInMag < MagazineSize)
		{
			Instance->CurrentAmmoInMag += 1;
			Instance->ReserveAmmo -= 1;
		}
	}
	else
	{
		const int32 Needed = MagazineSize - Instance->CurrentAmmoInMag;
		const int32 ToLoad = FMath::Min(Needed, Instance->ReserveAmmo);
		Instance->CurrentAmmoInMag += ToLoad;
		Instance->ReserveAmmo -= ToLoad;
	}
}

void UWeaponAmmoModule::TickCooling(UWeaponInstance* Instance, float DeltaSeconds) const
{
	if (!Instance || Model != EAmmoModel::Heat)
	{
		return;
	}

	if (Instance->bOverheated)
	{
		// Wait out the forced lockout before cooling resumes clearing it.
		Instance->OverheatTimer -= DeltaSeconds;
		if (Instance->OverheatTimer <= 0.0f)
		{
			Instance->OverheatTimer = 0.0f;
			Instance->bOverheated = false;
		}
	}

	if (Instance->CurrentHeat > 0.0f)
	{
		Instance->CurrentHeat = FMath::Max(0.0f, Instance->CurrentHeat - CoolingRate * DeltaSeconds);
	}
}
