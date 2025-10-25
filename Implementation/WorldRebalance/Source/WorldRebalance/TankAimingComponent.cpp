// Fill out your copyright notice in the Description page of Project Settings.


#include "TankAimingComponent.h"
#include "TankBarrel.h"
#include "TankProjectile.h"
#include "TankTurret.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/GameplayStaticsTypes.h" // 有些版本需要，这里包含 FSuggestProjectileVelocityParameters 的定义

// Sets default values for this component's properties
UTankAimingComponent::UTankAimingComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	
	PrimaryComponentTick.bCanEverTick = true;
}

void UTankAimingComponent::BeginPlay()
{
	Super::BeginPlay();
	//first fire is after initial reload (ReloadTime)
	LastFireTime = FPlatformTime::Seconds();
}

void UTankAimingComponent::TickComponent(float DeltaTime, enum ELevelTick TickType,
	FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	if (RoundsLeft <= 0)
	{
		FiringState = EFiringState::OutOfAmmo;
	}
	else if ((FPlatformTime::Seconds() - LastFireTime) < ReloadTime)
	{
		FiringState = EFiringState::Reloading;
	}
	else if (IsBarrelMoving())
	{
		FiringState = EFiringState::Aiming;
	}
	else
	{
		FiringState = EFiringState::Locked;
	}
}

int32 UTankAimingComponent::GetRoundsLeft() const
{
	return RoundsLeft;
}

EFiringState UTankAimingComponent::GetFiringState() const
{
	return FiringState;
}

bool UTankAimingComponent::IsBarrelMoving()
{
	if (!ensure(Barrel)){return false;}

	auto BarrelForward = Barrel->GetForwardVector();
	return !BarrelForward.Equals(AimDirection, 0.01);//vectors are equal
}

void UTankAimingComponent::Initialise(UTankBarrel* BarrelToSet, UTankTurret* TurretToSet)
{
	Barrel = BarrelToSet;
	Turret = TurretToSet;
}

void UTankAimingComponent::AimAt(FVector HitLocation)
{
	//炮臺指到天上就會落下到原位
	//https://chatgpt.com/s/t_68f6d957526c8191820acd30afe258f5
	if (!ensure(Barrel)){return;}

	FVector OutLaunchVelocity;
	FVector StartLocation = Barrel->GetSocketLocation(FName("Projectile"));
	//calculate the outlaunchVelocity
	bool bHaveAimSolution = UGameplayStatics::SuggestProjectileVelocity(
	GetOwner(),// <- 用 Actor(或其它 UObject) 作为 WorldContextObject，避免在头文件/签名解析时出现类型依赖问题
		OutLaunchVelocity,
		StartLocation,
		HitLocation,
		LaunchSpeed,
		false,
		0.0f,
		0.0f,
		ESuggestProjVelocityTraceOption::DoNotTrace
	);
	
	if (bHaveAimSolution)
	{
		AimDirection = OutLaunchVelocity.GetSafeNormal();
		MoveBarrelTowards(AimDirection);
	}
	else
	{
		//no solution found do nothing
	}
}

/*void UTankAimingComponent::MoveBarrelTowards(FVector AimDirection)
{
	if (!ensure(Barrel) || !ensure(Turret)){return;}
	
	//Work-out difference between current barrel reaction, and AimDirection
	auto BarrelRotator = Barrel->GetForwardVector().Rotation();
	auto AimAsRotator = AimDirection.Rotation();
	auto DeltaRotator = AimAsRotator - BarrelRotator;//----------------------------Aim delay
	//UE_LOG(LogTemp, Warning, TEXT("AimAsRotator: %s"), *DeltaRotator.ToString());
	
	Barrel->Elevate(DeltaRotator.Pitch);
	Turret->Rotate(DeltaRotator.Yaw);
}*/

void UTankAimingComponent::MoveBarrelTowards(FVector AimDirectionT)
{
	if (!ensure(Barrel) || !ensure(Turret)) { return; }

	// 當前與目標的旋轉
	const FRotator BarrelRotator = Barrel->GetForwardVector().Rotation();
	const FRotator AimAsRotator = AimDirectionT.Rotation();

	// 用 FindDeltaAngleDegrees 取得最短的角度差（會自動落在 -180..180）
	const float DeltaPitch = AimAsRotator.Pitch - BarrelRotator.Pitch; // pitch 通常不會有 wrap 問題，但這樣寫也可以
	const float DeltaYaw = FMath::FindDeltaAngleDegrees(BarrelRotator.Yaw, AimAsRotator.Yaw);

	Barrel->Elevate(DeltaPitch);
	Turret->Rotate(DeltaYaw);
}

void UTankAimingComponent::Fire()
{
	//bool isReloaded = (FPlatformTime::Seconds() - LastFireTime) > ReloadTime;
	
	if (FiringState == EFiringState::Locked || FiringState == EFiringState::Aiming)
	{
		//spawn projectile
		if (!ensure(Barrel)) {return;}
		if (!ensure(BPTankProjectile)) {return;}
		auto projectile = GetWorld()->SpawnActor<ATankProjectile>(
			BPTankProjectile,
			Barrel->GetSocketLocation(FName("Projectile")),
			Barrel->GetSocketRotation(FName("Projectile"))
			);

		projectile->LaunchProjectile(LaunchSpeed);
		LastFireTime = FPlatformTime::Seconds();
		RoundsLeft--;
	}
}
