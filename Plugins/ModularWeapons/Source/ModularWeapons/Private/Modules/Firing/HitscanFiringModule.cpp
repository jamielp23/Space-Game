// Copyright Continuum Saga. All Rights Reserved.

#include "Modules/Firing/HitscanFiringModule.h"

#include "Core/WeaponDefinition.h"
#include "Core/WeaponTypes.h"
#include "Modules/WeaponDamageModule.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraSystem.h"

void UHitscanFiringModule::ExecuteFire(const FWeaponFireContext& Context)
{
	if (!Context.World)
	{
		return;
	}

	PlayFireEffects(Context);

	// One shot may fire several traces (shotgun pellets).
	for (int32 PelletIndex = 0; PelletIndex < ProjectilesPerShot; ++PelletIndex)
	{
		FireSingleTrace(Context);
	}
}

void UHitscanFiringModule::FireSingleTrace(const FWeaponFireContext& Context) const
{
	const FVector Direction = ApplySpread(Context);
	const FVector Start = Context.ViewLocation;
	const FVector End = Start + Direction * MaxRange;

	FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(WeaponHitscan), /*bTraceComplex*/ true);
	QueryParams.AddIgnoredActor(Context.InstigatorPawn);
	QueryParams.bReturnPhysicalMaterial = true;

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

	// Tracer travels from the physical muzzle to the impact point.
	if (UNiagaraSystem* Tracer = TracerFX.LoadSynchronous())
	{
		if (UNiagaraComponent* TracerComp = UNiagaraFunctionLibrary::SpawnSystemAtLocation(Context.World, Tracer, Context.MuzzleLocation, Direction.Rotation()))
		{
			TracerComp->SetVectorParameter(TEXT("BeamEnd"), ImpactPoint);
		}
	}
}
