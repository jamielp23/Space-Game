// Copyright Continuum Saga. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "WeaponComponent.generated.h"

class UWeaponDefinition;
class UWeaponInstance;
class UMeshComponent;
struct FWeaponFireContext;

/** Fired when the equipped weapon changes (NewWeapon may be null when emptied). */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnEquippedWeaponChanged, UWeaponInstance*, NewWeapon);

/**
 * Drop-in weapon manager for any pawn (player or AI).
 *
 * Holds an inventory of runtime UWeaponInstance objects created from
 * UWeaponDefinition data assets, tracks the equipped weapon, forwards input
 * (fire / reload / aim / switch) to it, ticks it, and resolves the firing
 * geometry (view origin, aim direction, muzzle transform). The component is the
 * only piece that touches the owning actor, keeping every other system engine-
 * and actor-agnostic.
 */
UCLASS(ClassGroup = (Weapons), meta = (BlueprintSpawnableComponent))
class MODULARWEAPONS_API UWeaponComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UWeaponComponent();

	//~ Begin UActorComponent
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	//~ End UActorComponent

	// --- Inventory -------------------------------------------------------

	/** Weapons granted to the owner at spawn. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon|Loadout")
	TArray<TObjectPtr<UWeaponDefinition>> DefaultWeapons;

	/** Create a runtime instance from a definition and add it to the inventory. */
	UFUNCTION(BlueprintCallable, Category = "Weapon")
	UWeaponInstance* GrantWeapon(UWeaponDefinition* Definition, bool bEquipImmediately = false);

	/** Equip the weapon at an inventory index. */
	UFUNCTION(BlueprintCallable, Category = "Weapon")
	bool EquipWeaponAtIndex(int32 Index);

	/** Equip the next weapon in the inventory (wraps around). */
	UFUNCTION(BlueprintCallable, Category = "Weapon")
	void EquipNextWeapon();

	/** Equip the previous weapon in the inventory (wraps around). */
	UFUNCTION(BlueprintCallable, Category = "Weapon")
	void EquipPreviousWeapon();

	/** The currently equipped weapon, or null. */
	UFUNCTION(BlueprintPure, Category = "Weapon")
	UWeaponInstance* GetEquippedWeapon() const;

	/** All runtime weapons currently held. */
	UFUNCTION(BlueprintPure, Category = "Weapon")
	const TArray<TObjectPtr<UWeaponInstance>>& GetInventory() const { return Inventory; }

	// --- Input forwarding ------------------------------------------------

	UFUNCTION(BlueprintCallable, Category = "Weapon")
	void StartFire();

	UFUNCTION(BlueprintCallable, Category = "Weapon")
	void StopFire();

	UFUNCTION(BlueprintCallable, Category = "Weapon")
	void Reload();

	UFUNCTION(BlueprintCallable, Category = "Weapon")
	void SetAiming(bool bAiming);

	// --- Firing geometry -------------------------------------------------

	/**
	 * Fill a fire context with instigator, world, view origin, aim direction and
	 * muzzle transform. Marked virtual so projects can override how aim is
	 * resolved (e.g. AI aiming at a target rather than a camera).
	 */
	virtual void BuildFireContext(FWeaponFireContext& Context) const;

	/** Mesh whose socket provides the muzzle location; set by the owner. */
	UFUNCTION(BlueprintCallable, Category = "Weapon")
	void SetWeaponMesh(UMeshComponent* InMesh) { WeaponMesh = InMesh; }

	/** Socket name on the weapon mesh used as the muzzle. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
	FName MuzzleSocketName = TEXT("Muzzle");

	/**
	 * When true and the owner is controlled by a player, the component applies
	 * recoil directly to the control rotation. Disable to handle recoil yourself.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
	bool bApplyRecoilToController = true;

	/** Broadcast whenever the equipped weapon changes. */
	UPROPERTY(BlueprintAssignable, Category = "Weapon|Events")
	FOnEquippedWeaponChanged OnEquippedWeaponChanged;

protected:
	/** Bound to the equipped weapon's recoil event to apply view kick. */
	UFUNCTION()
	void HandleWeaponRecoil(UWeaponInstance* Weapon, FVector2D RecoilImpulse);

private:
	UPROPERTY()
	TArray<TObjectPtr<UWeaponInstance>> Inventory;

	UPROPERTY()
	TObjectPtr<UWeaponInstance> EquippedWeapon;

	UPROPERTY()
	TObjectPtr<UMeshComponent> WeaponMesh;

	int32 EquippedIndex = INDEX_NONE;

	/** Resolve the view point (origin + rotation) used for aiming. */
	void GetViewPoint(FVector& OutLocation, FRotator& OutRotation) const;
};
