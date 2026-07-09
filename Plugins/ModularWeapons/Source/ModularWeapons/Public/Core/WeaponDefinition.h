// Copyright Continuum Saga. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "Core/WeaponTypes.h"
#include "WeaponDefinition.generated.h"

class UWeaponFiringModule;
class UWeaponAmmoModule;
class UWeaponSpreadModule;
class UWeaponRecoilModule;
class UWeaponDamageModule;
class USkeletalMesh;
class UAnimMontage;

/**
 * The single authoring surface for a weapon — a Data Asset a designer creates
 * and configures entirely in the editor, no code required.
 *
 * A weapon is defined by composition: identity + presentation + firing cadence,
 * plus five independent, instanced behaviour modules (firing, ammo, spread,
 * recoil, damage). Any module may be left empty for a sensible default
 * (e.g. no spread module = perfect accuracy). This is what makes the whole
 * architecture modular: every weapon class in the game — pistol, rifle,
 * shotgun, sniper, energy, heavy, throwable — is just a different combination
 * of these modules, authored as data.
 */
UCLASS(BlueprintType)
class MODULARWEAPONS_API UWeaponDefinition : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	// --- Identity --------------------------------------------------------

	/** Player-facing weapon name. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Identity")
	FText DisplayName;

	/** Player-facing description / flavour text. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Identity", meta = (MultiLine = true))
	FText Description;

	/** Broad classification (drives UI / inventory / animation set selection). */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Identity")
	EWeaponType WeaponType = EWeaponType::Pistol;

	/** Gameplay tags for data-driven identification (e.g. Weapon.Faction.Unity). */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Identity")
	FGameplayTagContainer WeaponTags;

	// --- Presentation ----------------------------------------------------

	/** Third / first person weapon mesh. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Presentation")
	TSoftObjectPtr<USkeletalMesh> WeaponMesh;

	/** Fire animation played on the character each shot. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Presentation")
	TSoftObjectPtr<UAnimMontage> FireMontage;

	/** Reload animation played on the character. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Presentation")
	TSoftObjectPtr<UAnimMontage> ReloadMontage;

	/** Inventory / HUD icon. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Presentation")
	TSoftObjectPtr<UTexture2D> Icon;

	// --- Firing cadence --------------------------------------------------

	/** How the trigger maps to shots. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Firing")
	EWeaponFireMode FireMode = EWeaponFireMode::SingleShot;

	/** Rate of fire in rounds per minute (converted to a shot interval at runtime). */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Firing", meta = (ClampMin = "1.0"))
	float RoundsPerMinute = 450.0f;

	/** Shots fired per trigger pull when FireMode is Burst. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Firing", meta = (ClampMin = "1", EditCondition = "FireMode == EWeaponFireMode::Burst", EditConditionHides))
	int32 BurstCount = 3;

	/** Automatically start a reload when the magazine runs dry. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Firing")
	bool bAutoReloadOnEmpty = true;

	// --- Behaviour modules (composition) ---------------------------------

	/** Firing strategy: hitscan / projectile / beam / throwable. Required. */
	UPROPERTY(EditDefaultsOnly, Instanced, BlueprintReadOnly, Category = "Modules")
	TObjectPtr<UWeaponFiringModule> FiringModule;

	/** Resource model: magazine / heat / charges / infinite. */
	UPROPERTY(EditDefaultsOnly, Instanced, BlueprintReadOnly, Category = "Modules")
	TObjectPtr<UWeaponAmmoModule> AmmoModule;

	/** Accuracy / spread behaviour. Empty = perfect accuracy. */
	UPROPERTY(EditDefaultsOnly, Instanced, BlueprintReadOnly, Category = "Modules")
	TObjectPtr<UWeaponSpreadModule> SpreadModule;

	/** Recoil behaviour. Empty = no recoil. */
	UPROPERTY(EditDefaultsOnly, Instanced, BlueprintReadOnly, Category = "Modules")
	TObjectPtr<UWeaponRecoilModule> RecoilModule;

	/** Damage behaviour. Empty = projectiles carry their own damage only. */
	UPROPERTY(EditDefaultsOnly, Instanced, BlueprintReadOnly, Category = "Modules")
	TObjectPtr<UWeaponDamageModule> DamageModule;

	// --- Helpers ---------------------------------------------------------

	/** Seconds between shots derived from RoundsPerMinute. */
	UFUNCTION(BlueprintCallable, Category = "Weapon")
	float GetShotInterval() const;

	/** Validate that the definition has the minimum modules to function. */
	UFUNCTION(BlueprintCallable, Category = "Weapon")
	bool IsValidDefinition() const;

	//~ Begin UPrimaryDataAsset
	virtual FPrimaryAssetId GetPrimaryAssetId() const override;
	//~ End UPrimaryDataAsset

#if WITH_EDITOR
	//~ Validates modules against the chosen weapon type in the editor.
	virtual EDataValidationResult IsDataValid(class FDataValidationContext& Context) const override;
#endif
};
