#include "Tank.h"
#include "TankBarrel.h"
#include "TankTurret.h"
#include "TankProjectile.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"
#include "Net/UnrealNetwork.h"
#include "GameFramework/PlayerController.h"
#include "Math/UnrealMathUtility.h"

ATank::ATank()
{
    PrimaryActorTick.bCanEverTick = true;

    // Enable replication for multiplayer
    bReplicates = true;
    SetReplicates(true);
    SetReplicateMovement(true);
    CurrentHealth = StartingHealth;
}

void ATank::BeginPlay()
{
    Super::BeginPlay();


    LastFireTime = FPlatformTime::Seconds();
    IsDead = false;
    
    //FoundAimingComponent();
}

void ATank::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

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

    if (IsLocallyControlled())
    {
        AimTowardCrosshair();
    }
}

float ATank::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
    int32 DamagePoints = FPlatformMath::RoundToInt(DamageAmount);
    int32 DamageToApply = FMath::Clamp(DamagePoints, 0, CurrentHealth);

    isOnHit = true;
    CurrentHealth -= DamageToApply;
    GetDamaged += DamageToApply;
    
    //isOnHit = false;
    if (CurrentHealth <= 0)
    {
        IsDead = true;
        OnDeath.Broadcast();
    }

    //SetOnHit();

    return static_cast<float>(DamageToApply);
}

float ATank::GetHealthPercent() const
{
    return (StartingHealth > 0) ? (static_cast<float>(CurrentHealth) / static_cast<float>(StartingHealth)) : 0.0f;
}


float ATank::GetCurrentHealth() const
{
    return static_cast<float>(CurrentHealth);
}

float ATank::SetCurrentHealth(float SetHealth)
{
    if (!HasAuthority())
    {
        UE_LOG(LogTemp, Warning, TEXT("[SetCurrentHealth] Ignored on Client!"));
        return CurrentHealth;
    }
    
    int32 NewHealth = FMath::RoundToInt(SetHealth);
    NewHealth = FMath::Clamp(NewHealth, 0, StartingHealth);
    
    if (CurrentHealth != NewHealth)
    {
        CurrentHealth = NewHealth;
        UE_LOG(LogTemp, Log, TEXT("[Server] SetCurrentHealth -> %d"), CurrentHealth);
    }

    return static_cast<float>(CurrentHealth);
}

APC_Trooper* ATank::GetTankPlayerController() const
{
    return Cast<APC_Trooper>(GetOwner());
}

void ATank::Initialise(UTankBarrel* BarrelToSet, UTankTurret* TurretToSet)
{
    Barrel = BarrelToSet;
    Turret = TurretToSet;
}

void ATank::ServerAimAt_Implementation(const FVector& HitLocation)
{
    // 服务器端执行瞄准与真实旋转
    AimAt(HitLocation);
}

void ATank::AimAt(FVector HitLocation)
{
    if (!ensure(Barrel)) { return; }

    const FVector StartLocation = Barrel->GetSocketLocation(FName("Projectile"));

    if (HitLocation.IsZero())
    {
        HitLocation = StartLocation + Barrel->GetForwardVector() * LineTraceRange;
    }

    FVector OutLaunchVelocity;
    const bool bUseBallistic = true;

    if (bUseBallistic)
    {
        bool bHaveAimSolution = UGameplayStatics::SuggestProjectileVelocity(
            this,
            OutLaunchVelocity,
            StartLocation,
            HitLocation,
            LaunchSpeed,
            false,    // bHighArc = false（按需改）
            0.0f,     // CollisionRadius
            0.0f,     // Override gravity (0 使用默认)
            ESuggestProjVelocityTraceOption::DoNotTrace
        );

        if (bHaveAimSolution)
        {
            AimDirection = OutLaunchVelocity.GetSafeNormal();
        }
        else
        {
            AimDirection = (HitLocation - StartLocation).GetSafeNormal();
        }
    }
    else
    {
        AimDirection = (HitLocation - StartLocation).GetSafeNormal();
    }

    MoveBarrelTowards(AimDirection);
}

void ATank::MoveBarrelTowards(FVector AimDirectionT)
{
    if (!ensure(Barrel) || !ensure(Turret)) { return; }

    const FRotator BarrelRotator = Barrel->GetForwardVector().Rotation();
    const FRotator AimAsRotator = AimDirectionT.Rotation();

    const float DeltaPitch = AimAsRotator.Pitch - BarrelRotator.Pitch;
    const float DeltaYaw = FMath::FindDeltaAngleDegrees(BarrelRotator.Yaw, AimAsRotator.Yaw);

    Barrel->Elevate(DeltaPitch);
    Turret->Rotate(DeltaYaw);
}

bool ATank::IsBarrelMoving() const
{
    if (!ensure(Barrel)) { return false; }
    auto BarrelForward = Barrel->GetForwardVector();
    return !BarrelForward.Equals(AimDirection, 0.01f);
}

void ATank::Fire()
{
    if (!(FiringState == EFiringState::Locked || FiringState == EFiringState::Aiming))
    {
        return;
    }

    FVector HitLocation;
    bool bHasHit = GetSightRayHitLocation(HitLocation);

    if (!HasAuthority())
    {
        if (bHasHit)
        {
            ServerFire(HitLocation);

            //AimAt(HitLocation); // cosmetic only
        }
        else
        {
            ServerFire(FVector::ZeroVector);
        }
        return;
    }

    if (HasAuthority())
    {
        if (bHasHit)
        {
            ServerFire(HitLocation);
        }
        else
        {
            ServerFire(FVector::ZeroVector);
        }
    }
}

void ATank::ServerFire_Implementation(const FVector& HitLocation)
{
    if (!HasAuthority())
    {
        return;
    }

    if (!(FiringState == EFiringState::Locked || FiringState == EFiringState::Aiming))
    {
        return;
    }

    if (!ensure(Barrel) || !ensure(Turret) || !ensure(BPTankProjectile))
    {
        return;
    }

    FVector TargetLocation = HitLocation;
    if (TargetLocation.IsZero())
    {
        TargetLocation = Barrel->GetComponentLocation() + Barrel->GetForwardVector() * LineTraceRange;
    }

    FVector OutLaunchVelocity;
    const FVector StartLocation = Barrel->GetSocketLocation(FName("Projectile"));

    bool bHaveAimSolution = UGameplayStatics::SuggestProjectileVelocity(
        this,
        OutLaunchVelocity,
        StartLocation,
        TargetLocation,
        LaunchSpeed,
        false,
        0.0f,
        0.0f,
        ESuggestProjVelocityTraceOption::DoNotTrace
    );

    FVector AimDir;
    if (bHaveAimSolution)
    {
        AimDir = OutLaunchVelocity.GetSafeNormal();
    }
    else
    {
        AimDir = (TargetLocation - StartLocation).GetSafeNormal();
    }

    const FRotator AimRot = AimDir.Rotation();

    if (Turret)
    {
        FRotator TurretWorldRot = Turret->GetComponentRotation();
        TurretWorldRot.Yaw = AimRot.Yaw;
        Turret->SetWorldRotation(TurretWorldRot);
    }
    
    if (Barrel)
    {
        FRotator BarrelWorldRot = Barrel->GetComponentRotation();
        BarrelWorldRot.Pitch = AimRot.Pitch;
        Barrel->SetWorldRotation(BarrelWorldRot);
    }

    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }

    FActorSpawnParameters SpawnParams;
    SpawnParams.Owner = this;
    SpawnParams.Instigator = GetInstigator();
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

    ATankProjectile* Projectile = World->SpawnActor<ATankProjectile>(
        BPTankProjectile,
        StartLocation,
        AimRot,
        SpawnParams
    );

    if (Projectile)
    {
        Projectile->LaunchProjectile(LaunchSpeed);
        
        LastFireTime = FPlatformTime::Seconds();
        --RoundsLeft;
    }
}

void ATank::SetOnHit()
{
    if (isOnHit)
    {
        isOnHit = false;
    }
    else
    {
        isOnHit = true;
    }
}

void ATank::AimTowardCrosshair()
{
    APlayerController* PC = Cast<APlayerController>(GetController());
    if (!PC) { return; }

    FVector HitLocation;
    if (GetSightRayHitLocation(HitLocation))
    {
        if (!HasAuthority())
        {
            // 客户端：先做 cosmetic（本地显示），再请求服务器做真实旋转
            //AimAt(HitLocation);              // cosmetic: 让本地看起来即时响应
            ServerAimAt(HitLocation);        // 请求服务器权威执行（并由服务器同步给其他客户端）
        }
        else
        {
            // 服务器：直接执行
            AimAt(HitLocation);
        }
    }
}

bool ATank::GetSightRayHitLocation(FVector& HitLocation) const
{
    APlayerController* PC = Cast<APlayerController>(GetController());
    if (!PC) { return false; }

    int32 ViewportSizeX = 0, ViewportSizeY = 0;
    PC->GetViewportSize(ViewportSizeX, ViewportSizeY);

    FVector2D ScreenLocation(ViewportSizeX * 0.5f + CrosshairXLocation,
                             ViewportSizeY * 0.5f + CrosshairYLocation + 10);

    FVector LookDirection;
    if (GetLookDirection(ScreenLocation, LookDirection))
    {
        return GetLookVectorHitLocation(LookDirection, HitLocation);
    }
    return false;
}

bool ATank::GetLookDirection(FVector2D ScreenLocation, FVector& LookDirection) const
{
    APlayerController* PC = Cast<APlayerController>(GetController());
    if (!PC) { return false; }

    FVector CameraWorldLocation; // not used but required by Deproject
    return PC->DeprojectScreenPositionToWorld(ScreenLocation.X, ScreenLocation.Y, CameraWorldLocation, LookDirection);
}

bool ATank::GetLookVectorHitLocation(FVector LookDirection, FVector& HitLocation) const
{
    APlayerController* PC = Cast<APlayerController>(GetController());
    if (!PC) { HitLocation = FVector::ZeroVector; return false; }

    FVector StartLocation = PC->PlayerCameraManager->GetCameraLocation();
    FVector EndLocation = StartLocation + (LookDirection * LineTraceRange);

    FHitResult HitResult;
    if (GetWorld()->LineTraceSingleByChannel(HitResult, StartLocation, EndLocation, ECC_Visibility))
    {
        HitLocation = HitResult.Location;
        return true;
    }

    HitLocation = FVector::ZeroVector;
    return false;
}

/* ---------------- Replication ---------------- */


void ATank::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(ATank, CurrentHealth);
    DOREPLIFETIME(ATank, RoundsLeft);
    
}
