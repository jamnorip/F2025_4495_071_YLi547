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

    CurrentHealth = StartingHealth;
}

void ATank::BeginPlay()
{
    Super::BeginPlay();

    LastFireTime = FPlatformTime::Seconds();

    // 通知蓝图：瞄准系统已可用（原先 FoundAimingComponent 用来传组件引用，这里仅通知）
    FoundAimingComponent();
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
        // 客户端：计算瞄向并节流发送给服务器（你已有逻辑）
        //AimTowardCrosshair();
        // 节流发送的逻辑保持

    }
}


float ATank::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
    int32 DamagePoints = FPlatformMath::RoundToInt(DamageAmount);
    int32 DamageToApply = FMath::Clamp(DamagePoints, 0, CurrentHealth);

    isOnHit = true;
    CurrentHealth -= DamageToApply;
    //isOnHit = false;
    if (CurrentHealth <= 0)
    {
        OnDeath.Broadcast();
    }

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

void ATank::Initialise(UTankBarrel* BarrelToSet, UTankTurret* TurretToSet)
{
    Barrel = BarrelToSet;
    Turret = TurretToSet;
}

void ATank::AimAt(FVector HitLocation)
{
    if (!ensure(Barrel)) { return; }

    // Barrel 发射起点（muzzle）
    const FVector StartLocation = Barrel->GetSocketLocation(FName("Projectile"));

    // 如果 HitLocation 是零（未命中），就直接使用 Barrel 前方一个远点
    if (HitLocation.IsZero())
    {
        HitLocation = StartLocation + Barrel->GetForwardVector() * LineTraceRange;
    }

    // 先尝试做真实弹道求解（与服务器使用相同参数）
    FVector OutLaunchVelocity;
    const bool bUseBallistic = true; // 如果你希望直线瞄准，改为 false

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
            // 没有解析解时 fallback 为直线方向
            AimDirection = (HitLocation - StartLocation).GetSafeNormal();
        }
    }
    else
    {
        // 直线瞄准（视觉上更贴合 crosshair）
        AimDirection = (HitLocation - StartLocation).GetSafeNormal();
    }

    // 立即用计算结果旋转（本地或服务器同样调用）
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

// Fire 入口：在客户端计算 HitLocation 并请求服务器发射
void ATank::Fire()
{
    // 先做本地的条件判断（例如弹药/状态）
    if (!(FiringState == EFiringState::Locked || FiringState == EFiringState::Aiming))
    {
        return;
    }

    // 取得瞄准点（仅当本地玩家 controller 有 viewport 时才可）
    FVector HitLocation;
    bool bHasHit = GetSightRayHitLocation(HitLocation);

    if (!HasAuthority())
    {
        if (bHasHit)
        {
            // 不要直接调用 AimAt(HitLocation) 导致与服务器不同步后被强制校正
            // 改为：先发请求给服务器（服务器会 authoritative 计算并 spawn）
            ServerFire(HitLocation);

            // 可选的本地视觉预测（必须使用与服务器相同的求解参数）
            // 这里我们用相同的 AimAt 实现来预测（保证 client 用与 server 同样的算法）
            AimAt(HitLocation); // cosmetic only
        }
        else
        {
            ServerFire(FVector::ZeroVector);
        }
        return;
    }

    // 如果是服务器本身（host），直接使用本地计算的 HitLocation 发射
    if (HasAuthority())
    {
        // Note: 对于 host（listen server）调用 Fire()，GetSightRayHitLocation 也会在该机上返回有效结果
        if (bHasHit)
        {
            ServerFire(HitLocation); // 可直接调用（也可以把 spawn 放在这里）
        }
        else
        {
            ServerFire(FVector::ZeroVector);
        }
    }
}

// Server RPC implementation: 接收客户端的 HitLocation，在服务器上 authoritative 地瞄准并 spawn projectile
void ATank::ServerFire_Implementation(const FVector& HitLocation)
{
    // 仅在服务器上执行
    if (!HasAuthority())
    {
        return;
    }

    // 权威性检查：弹药与状态
    if (!(FiringState == EFiringState::Locked || FiringState == EFiringState::Aiming))
    {
        return;
    }

    if (!ensure(Barrel) || !ensure(Turret) || !ensure(BPTankProjectile))
    {
        return;
    }

    // 1) 决定目标点（如果客户端没有给有效点，则用 Barrel 前方作为 fallback）
    FVector TargetLocation = HitLocation;
    if (TargetLocation.IsZero())
    {
        TargetLocation = Barrel->GetComponentLocation() + Barrel->GetForwardVector() * LineTraceRange;
    }

    // 2) 计算发射速度向量（服务器进行计算以保证一致性）
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
        // 没有解析解时用简单方向向量作为 fallback
        AimDir = (TargetLocation - StartLocation).GetSafeNormal();
    }

    // 3) 立即把服务器端的 Turret / Barrel 对齐到目标方向（避免客户端预测被旧朝向覆盖）
    const FRotator AimRot = AimDir.Rotation();

    // --- Turret: 以世界旋转设置 Yaw（只设置 Yaw 保持其它旋转分量） ---
    if (Turret)
    {
        FRotator TurretWorldRot = Turret->GetComponentRotation();
        TurretWorldRot.Yaw = AimRot.Yaw;
        Turret->SetWorldRotation(TurretWorldRot); // 若需要可改为 SetRelativeRotation 视层级而定
    }

    // --- Barrel: 设置 Pitch （Barrel 常为 Turret 的子组件，按需使用 Relative 或 World） ---
    if (Barrel)
    {
        // 方式 A：直接改 WorldRotation（通用但若层级复杂可能不精确）
        FRotator BarrelWorldRot = Barrel->GetComponentRotation();
        BarrelWorldRot.Pitch = AimRot.Pitch;
        Barrel->SetWorldRotation(BarrelWorldRot);

        // 方式 B（更常见）：改相对旋转（若 Barrel 挂在 Turret 上）
        // Barrel->SetRelativeRotation(FRotator(AimRot.Pitch, 0.0f, 0.0f));
    }

    // 4) Spawn Projectile（服务器 authoritative）
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
        AimRot, // 使用 AimDir 的旋转作为朝向
        SpawnParams
    );

    if (Projectile)
    {
        // 在服务器上设置 projectile 的速度 / 发射（必须在服务端做）
        Projectile->LaunchProjectile(LaunchSpeed);

        // 更新服务器端状态，复制给客户端
        LastFireTime = FPlatformTime::Seconds();
        --RoundsLeft;

        // 如果希望播放特效（音效/粒子）在所有客户端播放，调用 Multicast
        Multicast_OnFireEffects();
    }
}

// Multicast: 在所有客户端（和服务器）播放发射特效（仅做 VFX/SFX）
void ATank::Multicast_OnFireEffects_Implementation()
{
    // 在蓝图或 C++ 里播放音效/粒子/摄像机震动等
    // 例如：UGameplayStatics::SpawnEmitterAtLocation(...);
    // 注意：不要在这里去 spawn projectile（发射必须由服务器 spawn）
}
/* ---------------- Sight / viewport helpers ---------------- */

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
        AimAt(HitLocation);
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
