// Copyright Continuum Saga. All Rights Reserved.

#include "Projectiles/WeaponProjectile.h"

#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/DamageType.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraSystem.h"
#include "Sound/SoundBase.h"
#include "TimerManager.h"

AWeaponProjectile::AWeaponProjectile()
{
	PrimaryActorTick.bCanEverTick = false;

	CollisionComponent = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionComponent"));
	CollisionComponent->InitSphereRadius(6.0f);
	CollisionComponent->SetCollisionProfileName(TEXT("BlockAllDynamic"));
	CollisionComponent->SetNotifyRigidBodyCollision(true);
	CollisionComponent->CanCharacterStepUpOn = ECB_No;
	SetRootComponent(CollisionComponent);

	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
	MeshComponent->SetupAttachment(CollisionComponent);
	MeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	MovementComponent = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("MovementComponent"));
	MovementComponent->UpdatedComponent = CollisionComponent;
	MovementComponent->InitialSpeed = 6000.0f;
	MovementComponent->MaxSpeed = 12000.0f;
	MovementComponent->bRotationFollowsVelocity = true;
	MovementComponent->bShouldBounce = false;
	MovementComponent->ProjectileGravityScale = 0.0f;

	InitialLifeSpan = 8.0f;
}

void AWeaponProjectile::ConfigureProjectile(const FWeaponProjectileParams& InParams)
{
	Params = InParams;
}

void AWeaponProjectile::SetDamageInstigator(AController* InController, APawn* InInstigatorPawn)
{
	DamageInstigatorController = InController;
	DamageInstigatorPawn = InInstigatorPawn;
	SetInstigator(InInstigatorPawn);
}

void AWeaponProjectile::BeginPlay()
{
	Super::BeginPlay();

	CollisionComponent->OnComponentHit.AddDynamic(this, &AWeaponProjectile::OnProjectileHit);

	// Ignore the shooter so the projectile does not immediately detonate on them.
	if (DamageInstigatorPawn)
	{
		CollisionComponent->IgnoreActorWhenMoving(DamageInstigatorPawn, true);
	}

	if (Params.FuseTime > 0.0f)
	{
		GetWorldTimerManager().SetTimer(FuseTimerHandle, this, &AWeaponProjectile::OnFuseExpired, Params.FuseTime, false);
	}
}

void AWeaponProjectile::OnProjectileHit(UPrimitiveComponent* /*HitComp*/, AActor* OtherActor, UPrimitiveComponent* /*OtherComp*/, FVector /*NormalImpulse*/, const FHitResult& Hit)
{
	if (OtherActor == this || OtherActor == DamageInstigatorPawn)
	{
		return;
	}

	if (Params.bDetonateOnImpact)
	{
		Detonate(Hit);
	}
}

void AWeaponProjectile::OnFuseExpired()
{
	FHitResult SelfHit;
	SelfHit.ImpactPoint = GetActorLocation();
	SelfHit.Location = GetActorLocation();
	Detonate(SelfHit);
}

void AWeaponProjectile::Detonate(const FHitResult& Hit)
{
	if (bHasDetonated)
	{
		return;
	}
	bHasDetonated = true;

	GetWorldTimerManager().ClearTimer(FuseTimerHandle);

	const FVector DetonationLocation = Hit.ImpactPoint.IsZero() ? GetActorLocation() : Hit.ImpactPoint;
	const TSubclassOf<UDamageType> EffectiveType = Params.DamageType ? Params.DamageType : TSubclassOf<UDamageType>(UDamageType::StaticClass());

	if (Params.ExplosionRadius > 0.0f && Params.ExplosionDamage > 0.0f)
	{
		// Area weapon (rocket / grenade): radial falloff damage + physics impulse.
		TArray<AActor*> IgnoreActors;
		UGameplayStatics::ApplyRadialDamageWithFalloff(
			this,
			Params.ExplosionDamage,
			Params.ExplosionDamage * 0.25f,
			DetonationLocation,
			Params.ExplosionRadius * 0.25f,
			Params.ExplosionRadius,
			1.0f,
			EffectiveType,
			IgnoreActors,
			this,
			DamageInstigatorController);
	}
	else if (Params.ImpactDamage > 0.0f && Hit.GetActor())
	{
		// Single-target projectile (bolt / bullet).
		UGameplayStatics::ApplyPointDamage(
			Hit.GetActor(),
			Params.ImpactDamage,
			GetActorForwardVector(),
			Hit,
			DamageInstigatorController,
			this,
			EffectiveType);
	}

	if (UNiagaraSystem* FX = ImpactFX.LoadSynchronous())
	{
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(this, FX, DetonationLocation, GetActorRotation());
	}
	if (USoundBase* Sound = ImpactSound.LoadSynchronous())
	{
		UGameplayStatics::PlaySoundAtLocation(this, Sound, DetonationLocation);
	}

	ReceiveDetonate(Hit);
	Destroy();
}
