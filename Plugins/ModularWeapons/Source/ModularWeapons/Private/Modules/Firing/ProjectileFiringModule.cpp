// Copyright Continuum Saga. All Rights Reserved.

#include "Modules/Firing/ProjectileFiringModule.h"

#include "Core/WeaponTypes.h"
#include "GameFramework/ProjectileMovementComponent.h"

void UProjectileFiringModule::ExecuteFire(const FWeaponFireContext& Context)
{
	if (!Context.World || !ProjectileClass)
	{
		return;
	}

	PlayFireEffects(Context);

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = Context.InstigatorPawn;
	SpawnParams.Instigator = Context.InstigatorPawn;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	for (int32 ProjectileIndex = 0; ProjectileIndex < ProjectilesPerShot; ++ProjectileIndex)
	{
		const FVector Direction = ApplySpread(Context);
		const FTransform SpawnTransform(Direction.Rotation(), Context.MuzzleLocation);

		AWeaponProjectile* Projectile = Context.World->SpawnActorDeferred<AWeaponProjectile>(
			ProjectileClass, SpawnTransform, Context.InstigatorPawn, Context.InstigatorPawn,
			ESpawnActorCollisionHandlingMethod::AlwaysSpawn);

		if (!Projectile)
		{
			continue;
		}

		Projectile->ConfigureProjectile(ProjectileParams);
		Projectile->SetDamageInstigator(Context.InstigatorController, Context.InstigatorPawn);

		if (UProjectileMovementComponent* Movement = Projectile->FindComponentByClass<UProjectileMovementComponent>())
		{
			Movement->InitialSpeed = MuzzleSpeed;
			Movement->MaxSpeed = FMath::Max(MuzzleSpeed, Movement->MaxSpeed);
			Movement->ProjectileGravityScale = GravityScale;
			Movement->Velocity = Direction * MuzzleSpeed;
		}

		Projectile->FinishSpawning(SpawnTransform);
	}
}
