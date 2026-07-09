// Copyright Continuum Saga. All Rights Reserved.

#include "Modules/WeaponRecoilModule.h"

#include "Curves/CurveFloat.h"

FVector2D UWeaponRecoilModule::GetRecoilImpulse(int32 ShotIndex) const
{
	float VerticalScale = 1.0f;
	if (const FRichCurve* Curve = VerticalClimbOverShots.GetRichCurveConst())
	{
		if (Curve->GetNumKeys() > 0)
		{
			VerticalScale = Curve->Eval(static_cast<float>(ShotIndex), 1.0f);
		}
	}

	const float Pitch = VerticalKickDegrees * VerticalScale;

	// Base random horizontal spread biased toward the designer-chosen side.
	const float RandomYaw = FMath::FRandRange(-HorizontalKickDegrees, HorizontalKickDegrees);
	const float BiasYaw = HorizontalKickDegrees * HorizontalBias;
	const float Yaw = RandomYaw + BiasYaw;

	return FVector2D(Pitch, Yaw);
}
