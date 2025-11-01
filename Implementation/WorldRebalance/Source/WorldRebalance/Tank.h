#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "Tank.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FTankDelegate);

// Enum for aiming state (放在同一文件，供类内使用)
UENUM(BlueprintType)
enum class EFiringState : uint8 { Reloading, Aiming, Locked, OutOfAmmo };

// Forward declarations
class UTankBarrel;
class UTankTurret;
class ATankProjectile;

UCLASS()
class WORLDREBALANCE_API ATank : public APawn
{
    GENERATED_BODY()

public:
    ATank();

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // Damage / Health
    virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser) override;
    UFUNCTION(BlueprintPure, Category = "Health")
    float GetHealthPercent() const;
    UFUNCTION(BlueprintPure, Category = "Health")
    float GetCurrentHealth() const;

    // 初始化 Barrel / Turret（从关卡或蓝图里设）
    UFUNCTION(BlueprintCallable, Category = "Setup")
    void Initialise(UTankBarrel* BarrelToSet, UTankTurret* TurretToSet);

    // 瞄准 / 射击接口
    void AimAt(FVector HitLocation);

    // 服务器 RPC：接收客户端计算的瞄准位置并在服务器上执行发射
    UFUNCTION(Server, Reliable)
    void ServerFire(const FVector& HitLocation);

    // 保持 Fire 为入口函数（Blueprint/输入可直接调用）
    UFUNCTION(BlueprintCallable, Category = "Firing")
    void Fire();

    UFUNCTION(NetMulticast, Reliable)
    void Multicast_OnFireEffects(); // 用于播放投射器发射特效（声音/粒子），在客户端执行

    
    UFUNCTION(BlueprintCallable, Category = "Firing")
    EFiringState GetFiringState() const { return FiringState; }

    UFUNCTION(BlueprintCallable, Category = "Firing")
    int32 GetRoundsLeft() const { return RoundsLeft; }

    // Blueprint event: 通知 UI / HUD 瞄准系統已準備好（不带参数）
    UFUNCTION(BlueprintImplementableEvent, Category = "Setup")
    void FoundAimingComponent();

    // Death delegate
    FTankDelegate OnDeath;

protected:
    // State
    UPROPERTY(BlueprintReadOnly, Category = "State")
    EFiringState FiringState = EFiringState::Reloading;

    // For widget / animation when hit
    UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Health")
    bool isOnHit = false;

    UFUNCTION(BlueprintCallable, Category = "Firing")
    void SetOnHit();

private:
    // ---- Sight / crosshair helpers (在 Pawn 内实现)
    UFUNCTION(BlueprintCallable, Category="Aiming")
    void AimTowardCrosshair();
    
    bool GetSightRayHitLocation(FVector& HitLocation) const;
    bool GetLookDirection(FVector2D ScreenLocation, FVector& LookDirection) const;
    bool GetLookVectorHitLocation(FVector LookDirection, FVector& HitLocation) const;

    // Crosshair offsets (像你之前那样)
    UPROPERTY(EditAnywhere)
    float CrosshairXLocation = 0;
    UPROPERTY(EditAnywhere)
    float CrosshairYLocation = -75;
    UPROPERTY(EditAnywhere)
    float LineTraceRange = 1000000;

    // ---- Aiming implementation (原 AimingComponent 的功能都在这里)
    UFUNCTION(BlueprintCallable, Category="Aiming")
    void MoveBarrelTowards(FVector AimDirection);
    UFUNCTION(BlueprintCallable, Category="Aiming")
    bool IsBarrelMoving() const;

    UPROPERTY(VisibleAnywhere, Category = "Components")
    UTankBarrel* Barrel = nullptr;

    UPROPERTY(VisibleAnywhere, Category = "Components")
    UTankTurret* Turret = nullptr;

    FVector AimDirection = FVector::ZeroVector;

    // Projectile class
    UPROPERTY(EditDefaultsOnly, Category = "Setup")
    TSubclassOf<ATankProjectile> BPTankProjectile;

    // Firing properties
    UPROPERTY(EditDefaultsOnly, Category = "Firing")
    float LaunchSpeed = 90000.0f;

    // 标记为可复制
    UPROPERTY(EditDefaultsOnly, Replicated, Category = "Firing", meta = (ClampMin = "0"))
    int32 RoundsLeft = 30;

    UPROPERTY(EditDefaultsOnly, Category = "Firing")
    float ReloadTime = 1.0f;

    double LastFireTime = 0.0;

    // Health
    UPROPERTY(EditDefaultsOnly, Category = "Setup")
    int32 StartingHealth = 100;

    // 标记为可复制（如果你希望客户端也能通过复制看到 health）
    UPROPERTY(Replicated, VisibleAnywhere, Category = "Health")
    int32 CurrentHealth;

    // Helper: current aiming state helper already exposed above via FiringState

    // ---- helpers for replication
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
};
