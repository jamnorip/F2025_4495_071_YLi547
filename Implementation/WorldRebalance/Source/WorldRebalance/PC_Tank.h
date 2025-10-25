// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "PC_Tank.generated.h"

class ATank;
class UTankAimingComponent;

/**
 * 
 */
UCLASS()
class WORLDREBALANCE_API APC_Tank : public APlayerController
{
	GENERATED_BODY()

public:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

protected:
	UFUNCTION(BlueprintImplementableEvent, Category = "Setup")
	void FoundAimingComponent(UTankAimingComponent* AimCompRef);
	
	UFUNCTION(BlueprintImplementableEvent, Category = "Setup")
	void FoundTank(ATank* Tank);
	
private:
	//start the tank moving the barrel
	void AimTowardCrosshair();

	//return an out parameter, true if hit landscape
	bool GetSightRayHitLocation(FVector& HitLocation) const;

	UPROPERTY(EditAnywhere)
	float CrosshairXLocation = 0;
	UPROPERTY(EditAnywhere)
	float CrosshairYLocation = -75;
	UPROPERTY(EditAnywhere)
	float LineTraceRange = 1000000;

	bool GetLookDirection(FVector2D ScreenLocation, FVector& LookDirection) const;
	bool GetLookVectorHitLocation(FVector LookDirection, FVector& HitLocation) const;
};
