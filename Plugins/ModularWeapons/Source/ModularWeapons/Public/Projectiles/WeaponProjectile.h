// Copyright Continuum Saga. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "WeaponProjectile.generated.h"

class USphereComponent;
class UProjectileMovementComponent;
class UStaticMeshComponent;
class UNiagaraSystem;
class UDamageType;
class USoundBase;

/**
 * Spawn-time parameters for a projectile. Populated by a firing module and
 * pushed into the projectile between SpawnActorDeferred and FinishSpawning so a
 * single reusable projectile class serves bullets, rockets and grenades.
 */
USTRUCT(BlueprintType)
struct MODULARWEAPONS_API FWeaponProjectileParams
{
	GENERATED_BODY()

	/** Direct impact damage. */
	UPROPERTY(BlueprintReadWrite, Category = "Projectile")
	float ImpactDamage = 20.0f;

	/** Radius of area damage; <= 0 disables radial damage. */
	UPROPERTY(BlueprintReadWrite, Category = "Projectile")
	float ExplosionRadius = 0.0f;

	/** Damage dealt at the centre of the explosion (falls off to the edge). */
	UPROPERTY(BlueprintReadWrite, Category = "Projectile")
	float ExplosionDamage = 0.0f;

	/** Seconds before the projectile auto-detonates; <= 0 means never. */
	UPROPERTY(BlueprintReadWrite, Category = "Projectile")
	float FuseTime = 0.0f;

	/** If true the projectile detonates on first contact instead of resting. */
	UPROPERTY(BlueprintReadWrite, Category = "Projectile")
	bool bDetonateOnImpact = true;

	/** Radial impulse applied to physics bodies on detonation. */
	UPROPERTY(BlueprintReadWrite, Category = "Projectile")
	float ExplosionImpulse = 40000.0f;

	/** Damage type used for attribution. */
	UPROPERTY(BlueprintReadWrite, Category = "Projectile")
	TSubclassOf<UDamageType> DamageType;
};

/**
 * Generic, Blueprint-extendable projectile used by the projectile and
 * throwable firing modules. Handles movement, impact, optional timed fuse and
 * point / radial damage. Designers can subclass it in Blueprint to add bespoke
 * meshes, trails and detonation FX without touching C++.
 */
UCLASS(Blueprintable)
class MODULARWEAPONS_API AWeaponProjectile : public AActor
{
	GENERATED_BODY()

public:
	AWeaponProjectile();

	/** Push spawn parameters in before FinishSpawning. */
	void ConfigureProjectile(const FWeaponProjectileParams& InParams);

	/** Set the controller / pawn credited for kills this projectile causes. */
	void SetDamageInstigator(AController* InController, APawn* InInstigatorPawn);

	/** Niagara system spawned at the detonation point. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Projectile|FX")
	TSoftObjectPtr<UNiagaraSystem> ImpactFX;

	/** Sound played at the detonation point. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Projectile|FX")
	TSoftObjectPtr<USoundBase> ImpactSound;

protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	void OnProjectileHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

	/** Detonate: apply damage, play FX, and destroy the actor. */
	UFUNCTION(BlueprintCallable, Category = "Projectile")
	void Detonate(const FHitResult& Hit);

	/** Blueprint hook fired just before the actor is destroyed on detonation. */
	UFUNCTION(BlueprintImplementableEvent, Category = "Projectile", meta = (DisplayName = "On Detonate"))
	void ReceiveDetonate(const FHitResult& Hit);

	/** Spherical collision that drives movement sweeps and impact events. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Projectile")
	TObjectPtr<USphereComponent> CollisionComponent;

	/** Optional visible mesh. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Projectile")
	TObjectPtr<UStaticMeshComponent> MeshComponent;

	/** Ballistic movement (speed / gravity configured by the firing module). */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Projectile")
	TObjectPtr<UProjectileMovementComponent> MovementComponent;

	/** Resolved spawn parameters. */
	UPROPERTY(BlueprintReadOnly, Category = "Projectile")
	FWeaponProjectileParams Params;

private:
	void OnFuseExpired();

	UPROPERTY()
	TObjectPtr<AController> DamageInstigatorController;

	UPROPERTY()
	TObjectPtr<APawn> DamageInstigatorPawn;

	FTimerHandle FuseTimerHandle;
	bool bHasDetonated = false;
};
