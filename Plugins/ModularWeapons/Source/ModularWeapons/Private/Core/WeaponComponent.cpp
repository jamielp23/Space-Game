// Copyright Continuum Saga. All Rights Reserved.

#include "Core/WeaponComponent.h"

#include "Components/MeshComponent.h"
#include "Core/WeaponDefinition.h"
#include "Core/WeaponInstance.h"
#include "Core/WeaponTypes.h"
#include "GameFramework/Controller.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"

UWeaponComponent::UWeaponComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = true;
}

void UWeaponComponent::BeginPlay()
{
	Super::BeginPlay();

	// Materialise the default loadout into runtime instances.
	for (UWeaponDefinition* Definition : DefaultWeapons)
	{
		if (Definition)
		{
			GrantWeapon(Definition, /*bEquipImmediately*/ EquippedIndex == INDEX_NONE);
		}
	}
}

void UWeaponComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (EquippedWeapon)
	{
		EquippedWeapon->Tick(DeltaTime);
	}
}

UWeaponInstance* UWeaponComponent::GrantWeapon(UWeaponDefinition* Definition, bool bEquipImmediately)
{
	if (!Definition || !Definition->IsValidDefinition())
	{
		return nullptr;
	}

	UWeaponInstance* Instance = NewObject<UWeaponInstance>(this);
	Instance->Initialize(Definition, this);
	Inventory.Add(Instance);

	if (bEquipImmediately || EquippedIndex == INDEX_NONE)
	{
		EquipWeaponAtIndex(Inventory.Num() - 1);
	}

	return Instance;
}

bool UWeaponComponent::EquipWeaponAtIndex(int32 Index)
{
	if (!Inventory.IsValidIndex(Index))
	{
		return false;
	}
	if (Index == EquippedIndex)
	{
		return true;
	}

	// Holster the outgoing weapon.
	if (EquippedWeapon)
	{
		EquippedWeapon->StopFire();
		EquippedWeapon->OnRecoil.RemoveDynamic(this, &UWeaponComponent::HandleWeaponRecoil);
		EquippedWeapon->OnUnequipped();
	}

	EquippedIndex = Index;
	EquippedWeapon = Inventory[Index];

	// Bring up the incoming weapon.
	EquippedWeapon->OnRecoil.AddDynamic(this, &UWeaponComponent::HandleWeaponRecoil);
	EquippedWeapon->OnEquipped();

	OnEquippedWeaponChanged.Broadcast(EquippedWeapon);
	return true;
}

void UWeaponComponent::EquipNextWeapon()
{
	if (Inventory.Num() == 0)
	{
		return;
	}
	const int32 Next = (EquippedIndex + 1) % Inventory.Num();
	EquipWeaponAtIndex(Next);
}

void UWeaponComponent::EquipPreviousWeapon()
{
	if (Inventory.Num() == 0)
	{
		return;
	}
	const int32 Prev = (EquippedIndex - 1 + Inventory.Num()) % Inventory.Num();
	EquipWeaponAtIndex(Prev);
}

UWeaponInstance* UWeaponComponent::GetEquippedWeapon() const
{
	return EquippedWeapon;
}

void UWeaponComponent::StartFire()
{
	if (EquippedWeapon)
	{
		EquippedWeapon->StartFire();
	}
}

void UWeaponComponent::StopFire()
{
	if (EquippedWeapon)
	{
		EquippedWeapon->StopFire();
	}
}

void UWeaponComponent::Reload()
{
	if (EquippedWeapon)
	{
		EquippedWeapon->Reload();
	}
}

void UWeaponComponent::SetAiming(bool bAiming)
{
	if (EquippedWeapon)
	{
		EquippedWeapon->SetAiming(bAiming);
	}
}

void UWeaponComponent::GetViewPoint(FVector& OutLocation, FRotator& OutRotation) const
{
	const AActor* Owner = GetOwner();

	// Prefer the controlling controller's viewpoint (camera aim).
	if (const APawn* Pawn = Cast<APawn>(Owner))
	{
		if (AController* Controller = Pawn->GetController())
		{
			Controller->GetPlayerViewPoint(OutLocation, OutRotation);
			return;
		}
	}

	// Fallback: the owning actor's transform.
	if (Owner)
	{
		OutLocation = Owner->GetActorLocation();
		OutRotation = Owner->GetActorRotation();
	}
}

void UWeaponComponent::BuildFireContext(FWeaponFireContext& Context) const
{
	AActor* Owner = GetOwner();
	APawn* Pawn = Cast<APawn>(Owner);

	Context.InstigatorPawn = Pawn;
	Context.InstigatorController = Pawn ? Pawn->GetController() : nullptr;
	Context.OwnerComponent = const_cast<UWeaponComponent*>(this);

	FVector ViewLocation;
	FRotator ViewRotation;
	GetViewPoint(ViewLocation, ViewRotation);

	Context.ViewLocation = ViewLocation;
	Context.AimDirection = ViewRotation.Vector();

	// Muzzle: a socket on the weapon mesh if available, else the view point.
	if (WeaponMesh && WeaponMesh->DoesSocketExist(MuzzleSocketName))
	{
		Context.MuzzleLocation = WeaponMesh->GetSocketLocation(MuzzleSocketName);
		Context.MuzzleComponent = WeaponMesh;
	}
	else
	{
		Context.MuzzleLocation = ViewLocation;
		Context.MuzzleComponent = nullptr;
	}
}

void UWeaponComponent::HandleWeaponRecoil(UWeaponInstance* /*Weapon*/, FVector2D RecoilImpulse)
{
	if (!bApplyRecoilToController)
	{
		return;
	}

	// Apply the kick to the player's control rotation: X = pitch up, Y = yaw right.
	if (const APawn* Pawn = Cast<APawn>(GetOwner()))
	{
		if (APlayerController* PC = Cast<APlayerController>(Pawn->GetController()))
		{
			FRotator ControlRotation = PC->GetControlRotation();
			ControlRotation.Pitch += RecoilImpulse.X; // pitch up
			ControlRotation.Yaw += RecoilImpulse.Y;   // yaw right
			PC->SetControlRotation(ControlRotation);
		}
	}
}
