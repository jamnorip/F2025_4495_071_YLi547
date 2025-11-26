#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "Tank.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FTankDelegate);

// Enum for aiming state
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
    UFUNCTION(BlueprintCallable, Category = "Health")
    float SetCurrentHealth(float SetHealth);
    
    UFUNCTION(BlueprintCallable, Category = "Tank")
    APC_Trooper* GetTankPlayerController() const;

    UPROPERTY(BlueprintReadWrite, Category = "Health")
    float GetDamaged;
    
    UFUNCTION(BlueprintCallable, Category = "Setup")
    void Initialise(UTankBarrel* BarrelToSet, UTankTurret* TurretToSet);
    
    void AimAt(FVector HitLocation);
    
    UFUNCTION(Server, Reliable)
    void ServerFire(const FVector& HitLocation);

    UFUNCTION(BlueprintCallable, Category = "Firing")
    void Fire();

    
    UFUNCTION(BlueprintCallable, Category = "Firing")
    EFiringState GetFiringState() const { return FiringState; }

    UFUNCTION(BlueprintCallable, Category = "Firing")
    int32 GetRoundsLeft() const { return RoundsLeft; }
    
    //UFUNCTION(BlueprintImplementableEvent, Category = "Setup")
    //void FoundAimingComponent();

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

    UPROPERTY(BlueprintReadWrite, Category = "Health")
    bool IsDead;
    
    // Health
    UPROPERTY(BlueprintReadWrite, Category = "Health")
    int32 StartingHealth = 100;

private:
    UFUNCTION(BlueprintCallable, Category="Aiming")
    void AimTowardCrosshair();

    UFUNCTION(Server, Reliable)
    void ServerAimAt(const FVector& HitLocation);
    
    bool GetSightRayHitLocation(FVector& HitLocation) const;
    bool GetLookDirection(FVector2D ScreenLocation, FVector& LookDirection) const;
    bool GetLookVectorHitLocation(FVector LookDirection, FVector& HitLocation) const;
    
    UPROPERTY(EditAnywhere)
    float CrosshairXLocation = 0;
    UPROPERTY(EditAnywhere)
    float CrosshairYLocation = -75;
    UPROPERTY(EditAnywhere)
    float LineTraceRange = 1000000;
    
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
    int32 RoundsLeft = 50;

    UPROPERTY(EditDefaultsOnly, Category = "Firing")
    float ReloadTime = 1.0f;

    double LastFireTime = 0.0;
    
    UPROPERTY(Replicated, VisibleAnywhere, Category = "Health")
    int32 CurrentHealth;


    
    // Helper: current aiming state helper already exposed above via FiringState

    // ---- helpers for replication
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
};
