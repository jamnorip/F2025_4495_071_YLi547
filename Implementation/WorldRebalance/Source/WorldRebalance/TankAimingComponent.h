// Fixed version: constructor moved to public, keep forward declarations only in header to avoid circular includes.


#pragma once


#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "TankAimingComponent.generated.h"


// Enum for aiming state
UENUM()
enum class EFiringState : uint8 { Reloading, Aiming, Locked, OutOfAmmo };


// Forward Declaration
class UTankBarrel;
class UTankTurret;
class ATankProjectile;


// Hold parameters for barrel's properties and elevate method
UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class WORLDREBALANCE_API UTankAimingComponent : public UActorComponent
{
	GENERATED_BODY()


public:
	// Constructor should be public
	UTankAimingComponent();


	UFUNCTION(BlueprintCallable, Category = "Setup")
	void Initialise(UTankBarrel* BarrelToSet, UTankTurret* TurretToSet);


	void AimAt(FVector HitLocation);


	UFUNCTION(BlueprintCallable, Category = "Firing")
	void Fire();


	EFiringState GetFiringState() const;


	UFUNCTION(BlueprintCallable, Category = "Firing")
	int32 GetRoundsLeft() const;


protected:
	UPROPERTY(BlueprintReadOnly, Category = "State")
	EFiringState FiringState = EFiringState::Reloading;


private:
	virtual void BeginPlay() override;


	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;


	void MoveBarrelTowards(FVector AimDirection);


	bool IsBarrelMoving();


	UTankBarrel* Barrel = nullptr;
	UTankTurret* Turret = nullptr;


	UPROPERTY(EditDefaultsOnly, Category = "Setup")
	TSubclassOf<ATankProjectile> BPTankProjectile;


	UPROPERTY(EditDefaultsOnly, Category = "Firing")
	float LaunchSpeed = 90000; // 900m/s


	UPROPERTY(EditDefaultsOnly, Category = "Firing")
	int32 RoundsLeft = 30;


	UPROPERTY(EditDefaultsOnly, Category = "Firing")
	float ReloadTime = 1;


	double LastFireTime = 0;


	FVector AimDirection;
};