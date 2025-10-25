// Fill out your copyright notice in the Description page of Project Settings.

#include "PC_Tank.h"
#include "TankAimingComponent.h"
#include "Tank.h" 

void APC_Tank::BeginPlay()
{
	Super::BeginPlay();
	
	// 检查 Pawn 是否存在
	APawn* MyPawn = GetPawn();
	if (!MyPawn)
	{
		UE_LOG(LogTemp, Warning, TEXT("PC_Tank: No pawn possessed yet."));
		return;
	}

	// ✅ 取得瞄准组件
	auto AimingComponent = MyPawn->FindComponentByClass<UTankAimingComponent>();
	if (!ensure(AimingComponent)) { return; }
	FoundAimingComponent(AimingComponent);

	// ✅ 取得 Tank 本身（Actor）
	auto Tank = Cast<ATank>(MyPawn);
	if (!ensure(Tank)) { return; }
	FoundTank(Tank);
}

//Tick AimTowardCrosshair
void APC_Tank::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	AimTowardCrosshair();
}


void APC_Tank::AimTowardCrosshair()
{
	if (!GetPawn()) {return;}
	
	auto AimingComponent = GetPawn()->FindComponentByClass<UTankAimingComponent>();
	if (!ensure(AimingComponent)) {return;}
	
	FVector HitLocation;
	
	//get world location if linetrace through crosshair
	bool bGotHitLocation = GetSightRayHitLocation(HitLocation);
	if (bGotHitLocation) //has "side-effect", is going to line trace
	{
		AimingComponent->AimAt(HitLocation);
	}
}

bool APC_Tank::GetSightRayHitLocation(FVector& HitLocation) const
{
	//Find crosshair pos
	int32 ViewportSizeX, ViewportSizeY;
	GetViewportSize(ViewportSizeX, ViewportSizeY);
	FVector2D ScreenLocation(ViewportSizeX * 0.5f + CrosshairXLocation,
							 ViewportSizeY * 0.5f + CrosshairYLocation + 10);//has lower than real compiled...
	//UE_LOG(LogTemp, Warning, TEXT("Screen Location: %s"), *ScreenLocation.ToString());

	//DeprojectScreenPositionToWorld
	FVector LookDirection;
	if (GetLookDirection(ScreenLocation, LookDirection))
	{
		//linetrace
		//UE_LOG(LogTemp, Warning, TEXT("Look direction: %s"), *LookDirection.ToString());
		return GetLookVectorHitLocation(LookDirection, HitLocation);
	}

	return false;
}

bool APC_Tank::GetLookVectorHitLocation(FVector LookDirection, FVector& HitLocation) const
{
	FHitResult HitResult;
	auto StartLocation = PlayerCameraManager->GetCameraLocation();
	auto EndLocation = StartLocation + (LookDirection * LineTraceRange);//CrosshairYLocation

	if (GetWorld()->LineTraceSingleByChannel(HitResult, StartLocation, EndLocation, ECC_Visibility))//if line trace succeed
	{
		//set hit location
		HitLocation = HitResult.Location;
		return true;
	}
	HitLocation = FVector::ZeroVector;
	return false;
}

bool APC_Tank::GetLookDirection(FVector2D ScreenLocation, FVector& LookDirection) const
{
	FVector CameraWorldLocation;
	return DeprojectScreenPositionToWorld(ScreenLocation.X, ScreenLocation.Y, CameraWorldLocation, LookDirection);
}