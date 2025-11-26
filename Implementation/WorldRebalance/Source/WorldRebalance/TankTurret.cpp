// Fill out your copyright notice in the Description page of Project Settings.


#include "TankTurret.h"

UTankTurret::UTankTurret()
{
	PrimaryComponentTick.bCanEverTick = false;

	// 组件复制开启
	SetIsReplicatedByDefault(true);
}

void UTankTurret::Rotate(float RelativeSpeed)
{
	FRotator CurrentRotation = GetRelativeRotation();
	RelativeSpeed = FMath::Clamp<float>(RelativeSpeed, -1.0f, +1.0f);
	auto RotationChange = RelativeSpeed * MaxDegreesPerSecond * GetWorld()->DeltaTimeSeconds;
	auto Rotation = CurrentRotation.Yaw + RotationChange;
	//auto Rotation = FMath::Clamp<float>(RawNewRotation, MinRotationDegrees, MaxRotationDegrees);
	SetRelativeRotation(FRotator(0,Rotation , 0));
}