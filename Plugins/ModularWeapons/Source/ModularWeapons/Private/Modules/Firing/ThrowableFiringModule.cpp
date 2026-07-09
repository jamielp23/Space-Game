// Copyright Continuum Saga. All Rights Reserved.

#include "Modules/Firing/ThrowableFiringModule.h"

#include "Core/WeaponTypes.h"
#include "GameFramework/ProjectileMovementComponent.h"

void UThrowableFiringModule::ExecuteFire(const FWeaponFireContext& Context)
{
	if (!Context.World || !ThrowableClass)
	{
		return;
	}

	PlayFireEffects(Context);

	// Build an arcing launch direction: aim, lifted toward world-up.
	const FVector AimDir = Context.AimDirection.GetSafeNormal();
	const FVector ThrowDir = (AimDir + FVector::UpVector * UpwardArc).GetSafeNormal();
	const FTransform SpawnTransform(ThrowDir.Rotation(), Context.MuzzleLocation);

	AWeaponProjectile* Throwable = Context.World->SpawnActorDeferred<AWeaponProjectile>(
		ThrowableClass, SpawnTransform, Context.InstigatorPawn, Context.InstigatorPawn,
		ESpawnActorCollisionHandlingMethod::AlwaysSpawn);

	if (!Throwable)
	{
		return;
	}

	Throwable->ConfigureProjectile(ThrowableParams);
	Throwable->SetDamageInstigator(Context.InstigatorController, Context.InstigatorPawn);

	if (UProjectileMovementComponent* Movement = Throwable->FindComponentByClass<UProjectileMovementComponent>())
	{
		Movement->InitialSpeed = ThrowSpeed;
		Movement->MaxSpeed = FMath::Max(ThrowSpeed * 2.0f, Movement->MaxSpeed);
		Movement->ProjectileGravityScale = GravityScale;
		Movement->bShouldBounce = true;
		Movement->Bounciness = 0.3f;
		Movement->Velocity = ThrowDir * ThrowSpeed;
	}

	Throwable->FinishSpawning(SpawnTransform);
}
