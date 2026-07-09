// Copyright Continuum Saga. All Rights Reserved.

#include "Modules/Firing/WeaponFiringModule.h"

#include "Core/WeaponDefinition.h"
#include "Core/WeaponTypes.h"
#include "Modules/WeaponSpreadModule.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraSystem.h"
#include "Sound/SoundBase.h"

void UWeaponFiringModule::PlayFireEffects(const FWeaponFireContext& Context) const
{
	if (UNiagaraSystem* FX = MuzzleFX.LoadSynchronous())
	{
		if (Context.MuzzleComponent)
		{
			UNiagaraFunctionLibrary::SpawnSystemAttached(
				FX, Context.MuzzleComponent, MuzzleSocketName, FVector::ZeroVector,
				FRotator::ZeroRotator, EAttachLocation::SnapToTarget, true);
		}
		else if (Context.World)
		{
			UNiagaraFunctionLibrary::SpawnSystemAtLocation(
				Context.World, FX, Context.MuzzleLocation, Context.AimDirection.Rotation());
		}
	}

	if (USoundBase* Sound = FireSound.LoadSynchronous())
	{
		UGameplayStatics::PlaySoundAtLocation(Context.World, Sound, Context.MuzzleLocation);
	}
}

FVector UWeaponFiringModule::ApplySpread(const FWeaponFireContext& Context) const
{
	const FVector BaseDir = Context.AimDirection.GetSafeNormal();

	if (Context.Definition)
	{
		if (const UWeaponSpreadModule* Spread = Context.Definition->SpreadModule)
		{
			return Spread->GetSpreadDirection(Context, BaseDir);
		}
	}
	return BaseDir;
}
