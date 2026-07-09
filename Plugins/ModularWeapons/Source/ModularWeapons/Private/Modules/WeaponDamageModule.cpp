// Copyright Continuum Saga. All Rights Reserved.

#include "Modules/WeaponDamageModule.h"

#include "Core/WeaponTypes.h"
#include "GameFramework/DamageType.h"
#include "Kismet/GameplayStatics.h"

bool UWeaponDamageModule::IsCriticalHit(const FHitResult& Hit) const
{
	if (Hit.BoneName == NAME_None || CriticalHitBones.Num() == 0)
	{
		return false;
	}
	return CriticalHitBones.Contains(Hit.BoneName);
}

float UWeaponDamageModule::ComputeDamage(float Distance, const FHitResult& Hit) const
{
	float Damage = BaseDamage;

	// Distance falloff: an empty curve evaluates to the supplied default (1.0),
	// meaning "no falloff" until a designer authors keys.
	if (const FRichCurve* Curve = DamageFalloffOverDistance.GetRichCurveConst())
	{
		if (Curve->GetNumKeys() > 0)
		{
			Damage *= Curve->Eval(Distance, 1.0f);
		}
	}

	if (IsCriticalHit(Hit))
	{
		Damage *= CriticalHitMultiplier;
	}

	return FMath::Max(0.0f, Damage);
}

float UWeaponDamageModule::ApplyPointDamage(const FWeaponFireContext& Context, const FHitResult& Hit) const
{
	AActor* HitActor = Hit.GetActor();
	if (!HitActor)
	{
		return 0.0f;
	}

	const float Distance = FVector::Dist(Context.MuzzleLocation, Hit.ImpactPoint);
	const float FinalDamage = ComputeDamage(Distance, Hit);
	if (FinalDamage <= 0.0f)
	{
		return 0.0f;
	}

	const TSubclassOf<UDamageType> EffectiveType = DamageType ? DamageType : TSubclassOf<UDamageType>(UDamageType::StaticClass());

	UGameplayStatics::ApplyPointDamage(
		HitActor,
		FinalDamage,
		Context.AimDirection,
		Hit,
		Context.InstigatorController,
		Context.InstigatorPawn,
		EffectiveType);

	return FinalDamage;
}
