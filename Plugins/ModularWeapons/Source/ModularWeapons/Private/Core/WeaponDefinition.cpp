// Copyright Continuum Saga. All Rights Reserved.

#include "Core/WeaponDefinition.h"

#include "Modules/Firing/WeaponFiringModule.h"
#include "Modules/WeaponAmmoModule.h"

#if WITH_EDITOR
#include "Misc/DataValidation.h"
#endif

#define LOCTEXT_NAMESPACE "WeaponDefinition"

float UWeaponDefinition::GetShotInterval() const
{
	// RoundsPerMinute is clamped >= 1, so this is always safe.
	return 60.0f / FMath::Max(1.0f, RoundsPerMinute);
}

bool UWeaponDefinition::IsValidDefinition() const
{
	// A weapon must at minimum know how to emit a shot.
	return FiringModule != nullptr;
}

FPrimaryAssetId UWeaponDefinition::GetPrimaryAssetId() const
{
	return FPrimaryAssetId(FPrimaryAssetType(TEXT("Weapon")), GetFName());
}

#if WITH_EDITOR
EDataValidationResult UWeaponDefinition::IsDataValid(FDataValidationContext& Context) const
{
	EDataValidationResult Result = Super::IsDataValid(Context);

	if (!FiringModule)
	{
		Context.AddError(LOCTEXT("MissingFiringModule", "Weapon definition requires a Firing Module."));
		Result = EDataValidationResult::Invalid;
	}

	if (FireMode == EWeaponFireMode::Burst && BurstCount < 1)
	{
		Context.AddError(LOCTEXT("BadBurstCount", "Burst fire mode requires a Burst Count of at least 1."));
		Result = EDataValidationResult::Invalid;
	}

	if (WeaponType == EWeaponType::Throwable && !AmmoModule)
	{
		Context.AddWarning(LOCTEXT("ThrowableNoAmmo", "Throwable weapons usually use a Charges ammo module to limit throws."));
	}

	return Result;
}
#endif

#undef LOCTEXT_NAMESPACE
