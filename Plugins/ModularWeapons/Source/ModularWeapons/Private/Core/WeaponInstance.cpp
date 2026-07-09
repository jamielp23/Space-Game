// Copyright Continuum Saga. All Rights Reserved.

#include "Core/WeaponInstance.h"

#include "Core/WeaponComponent.h"
#include "Core/WeaponDefinition.h"
#include "ModularWeapons.h"
#include "Modules/Firing/WeaponFiringModule.h"
#include "Modules/WeaponAmmoModule.h"
#include "Modules/WeaponRecoilModule.h"
#include "Modules/WeaponSpreadModule.h"

void UWeaponInstance::Initialize(UWeaponDefinition* InDefinition, UWeaponComponent* InOwnerComponent)
{
	Definition = InDefinition;
	OwnerComponent = InOwnerComponent;

	if (Definition && Definition->AmmoModule)
	{
		Definition->AmmoModule->InitializeState(this);
	}

	SetState(EWeaponState::Idle);
	NotifyResourceChanged();
}

void UWeaponInstance::StartFire()
{
	if (!Definition || !Definition->FiringModule)
	{
		return;
	}

	bTriggerHeld = true;

	switch (Definition->FireMode)
	{
	case EWeaponFireMode::SingleShot:
		TryFireOnce();
		break;

	case EWeaponFireMode::Burst:
		if (!bBurstActive)
		{
			bBurstActive = true;
			BurstShotsRemaining = FMath::Max(1, Definition->BurstCount);
			TryFireOnce(); // Tick fires the rest on cadence.
		}
		break;

	case EWeaponFireMode::FullAuto:
		TryFireOnce(); // Tick sustains the rest on cadence.
		break;

	default:
		break;
	}
}

void UWeaponInstance::StopFire()
{
	bTriggerHeld = false;
	SustainedShotIndex = 0;

	if (State == EWeaponState::Firing)
	{
		SetState(EWeaponState::Idle);
	}
}

void UWeaponInstance::Reload()
{
	if (!Definition || !Definition->AmmoModule)
	{
		return;
	}
	if (State == EWeaponState::Reloading || State == EWeaponState::Overheated)
	{
		return;
	}
	if (!Definition->AmmoModule->CanReload(this))
	{
		return;
	}

	ReloadTimeRemaining = Definition->AmmoModule->GetReloadDuration(this);
	SetState(EWeaponState::Reloading);
}

void UWeaponInstance::SetAiming(bool bNewAiming)
{
	bIsAiming = bNewAiming;
}

void UWeaponInstance::Tick(float DeltaSeconds)
{
	TimeSinceLastShot += DeltaSeconds;

	// Passive systems always advance.
	if (Definition)
	{
		if (Definition->AmmoModule)
		{
			const bool bWasOverheated = bOverheated;
			Definition->AmmoModule->TickCooling(this, DeltaSeconds);
			if (bWasOverheated && !bOverheated && State == EWeaponState::Overheated)
			{
				SetState(EWeaponState::Idle);
			}
			OnHeatChanged.Broadcast(this, GetHeatPercent());
		}

		if (Definition->SpreadModule)
		{
			Definition->SpreadModule->TickRecovery(this, DeltaSeconds);
		}
	}

	// Drive an in-progress reload.
	if (State == EWeaponState::Reloading)
	{
		ReloadTimeRemaining -= DeltaSeconds;
		if (ReloadTimeRemaining <= 0.0f)
		{
			FinishReload();
		}
		return; // No firing while reloading.
	}

	// Reflect an overheat that occurred on the last shot.
	if (bOverheated && State != EWeaponState::Overheated)
	{
		SetState(EWeaponState::Overheated);
	}

	// Sustained fire: full-auto while held, or finishing a burst.
	if (Definition)
	{
		const bool bFullAutoHeld = bTriggerHeld && Definition->FireMode == EWeaponFireMode::FullAuto;
		const bool bBurstContinuing = bBurstActive && BurstShotsRemaining > 0;
		if (bFullAutoHeld || bBurstContinuing)
		{
			TryFireOnce();
		}
	}
}

void UWeaponInstance::OnEquipped()
{
	SetState(EWeaponState::Idle);
	NotifyResourceChanged();
}

void UWeaponInstance::OnUnequipped()
{
	StopFire();
	// Cancel any in-progress reload; it restarts fresh next equip.
	if (State == EWeaponState::Reloading)
	{
		ReloadTimeRemaining = 0.0f;
		SetState(EWeaponState::Idle);
	}
}

float UWeaponInstance::GetHeatPercent() const
{
	if (!Definition || !Definition->AmmoModule || Definition->AmmoModule->Model != EAmmoModel::Heat)
	{
		return 0.0f;
	}
	const float MaxHeat = FMath::Max(1.0f, Definition->AmmoModule->MaxHeat);
	return FMath::Clamp(CurrentHeat / MaxHeat, 0.0f, 1.0f);
}

bool UWeaponInstance::TryFireOnce()
{
	if (!Definition || !Definition->FiringModule)
	{
		return false;
	}

	// Cannot fire while busy.
	if (State == EWeaponState::Reloading || State == EWeaponState::Overheated || State == EWeaponState::Equipping)
	{
		return false;
	}

	// Respect the fire cadence.
	if (TimeSinceLastShot < Definition->GetShotInterval())
	{
		return false;
	}

	// Respect the resource model (ammo / heat / charges).
	if (Definition->AmmoModule && !Definition->AmmoModule->CanFire(this))
	{
		// Out of ammo: auto-reload if configured, else just click empty.
		if (Definition->bAutoReloadOnEmpty && Definition->AmmoModule->CanReload(this))
		{
			Reload();
		}
		return false;
	}

	FireShot();
	return true;
}

void UWeaponInstance::FireShot()
{
	// Resolve the firing geometry from the owning component.
	FWeaponFireContext Context;
	Context.Instance = this;
	Context.Definition = Definition;
	Context.OwnerComponent = OwnerComponent;
	Context.World = OwnerComponent ? OwnerComponent->GetWorld() : nullptr;
	Context.ShotIndex = SustainedShotIndex;

	if (OwnerComponent)
	{
		OwnerComponent->BuildFireContext(Context);
	}

	if (!Context.World)
	{
		UE_LOG(LogModularWeapons, Warning, TEXT("WeaponInstance::FireShot aborted: no world available."));
		return;
	}

	// 1. Emit the shot through the firing strategy.
	Definition->FiringModule->ExecuteFire(Context);

	// 2. Consume the resource cost.
	if (Definition->AmmoModule)
	{
		Definition->AmmoModule->ConsumeForShot(this);
	}

	// 3. Grow spread bloom.
	if (Definition->SpreadModule)
	{
		Definition->SpreadModule->OnShotFired(this);
	}

	// 4. Emit recoil for the owner to consume.
	if (Definition->RecoilModule)
	{
		const FVector2D Impulse = Definition->RecoilModule->GetRecoilImpulse(SustainedShotIndex);
		OnRecoil.Broadcast(this, Impulse);
	}

	// 5. Advance cadence / burst / pattern bookkeeping.
	TimeSinceLastShot = 0.0f;
	++SustainedShotIndex;

	if (bBurstActive)
	{
		--BurstShotsRemaining;
		if (BurstShotsRemaining <= 0)
		{
			bBurstActive = false;
		}
	}

	// 6. Notify listeners.
	SetState(EWeaponState::Firing);
	OnFired.Broadcast(this);
	NotifyResourceChanged();

	if (bOverheated)
	{
		SetState(EWeaponState::Overheated);
	}
}

void UWeaponInstance::FinishReload()
{
	if (!Definition || !Definition->AmmoModule)
	{
		SetState(EWeaponState::Idle);
		return;
	}

	Definition->AmmoModule->CompleteReload(this);
	NotifyResourceChanged();

	// Shell-by-shell reloads keep going until full or out of reserve.
	if (Definition->AmmoModule->bReloadPerRound && Definition->AmmoModule->CanReload(this))
	{
		ReloadTimeRemaining = Definition->AmmoModule->GetReloadDuration(this);
		// Remain in the Reloading state for the next shell.
		return;
	}

	SetState(EWeaponState::Idle);
}

void UWeaponInstance::SetState(EWeaponState NewState)
{
	if (State == NewState)
	{
		return;
	}
	State = NewState;
	OnStateChanged.Broadcast(this, State);
}

void UWeaponInstance::NotifyResourceChanged()
{
	OnAmmoChanged.Broadcast(this, CurrentAmmoInMag, ReserveAmmo);
	OnHeatChanged.Broadcast(this, GetHeatPercent());
}
