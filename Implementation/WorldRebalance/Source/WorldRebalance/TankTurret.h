// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/StaticMeshComponent.h"
#include "TankTurret.generated.h"

/**
 * 
 */
UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class WORLDREBALANCE_API UTankTurret : public UStaticMeshComponent
{
	GENERATED_BODY()

public:
	//-1 is max downward speed, +1 is max up movement
	void Rotate(float RelativeSpeed);

private:
	UPROPERTY(EditDefaultsOnly, Category="Setup")
	float MaxDegreesPerSecond = 5;
};
