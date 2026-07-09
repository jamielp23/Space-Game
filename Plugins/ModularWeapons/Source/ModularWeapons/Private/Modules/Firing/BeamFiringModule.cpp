// Copyright Continuum Saga. All Rights Reserved.

#include "Modules/Firing/BeamFiringModule.h"

#include "Core/WeaponDefinition.h"
#include "Core/WeaponTypes.h"
#include "Modules/WeaponDamageModule.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraSystem.h"

void UBeamFiringModule::ExecuteFire(const FWeaponFireContext& Context)
{
	if (!Context.World)
	{
		return;
	}

	PlayFireEffects(Context);

	const FVector Direction = ApplySpread(Context);
	const FVector Start = Context.ViewLocation;
	const FVector End = Start + Direction * MaxRange;

	FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(WeaponBeam), /*bTraceComplex*/ true);
	QueryParams.AddIgnoredActor(Context.InstigatorPawn);

	FHitResult Hit;
	const bool bHit = Context.World->LineTraceSingleByChannel(Hit, Start, End, TraceChannel, QueryParams);
	const FVector ImpactPoint = bHit ? Hit.ImpactPoint : End;

	if (bHit)
	{
		if (Context.Definition)
		{
			if (const UWeaponDamageModule* Damage = Context.Definition->DamageModule)
			{
				Damage->ApplyPointDamage(Context, Hit);
			}
		}

		if (UNiagaraSystem* FX = ImpactFX.LoadSynchronous())
		{
			UNiagaraFunctionLibrary::SpawnSystemAtLocation(Context.World, FX, ImpactPoint, Hit.ImpactNormal.Rotation());
		}
	}

	// Render the beam from the physical muzzle to the impact point.
	if (UNiagaraSystem* Beam = BeamFX.LoadSynchronous())
	{
		if (UNiagaraComponent* BeamComp = UNiagaraFunctionLibrary::SpawnSystemAtLocation(Context.World, Beam, Context.MuzzleLocation, Direction.Rotation()))
		{
			BeamComp->SetVectorParameter(TEXT("BeamEnd"), ImpactPoint);
		}
	}
}
