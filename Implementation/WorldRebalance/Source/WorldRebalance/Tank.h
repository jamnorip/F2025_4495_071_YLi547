// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "TankTrack.h"
#include "GameFramework/Pawn.h"
#include "Tank.generated.h"

UCLASS()
class WORLDREBALANCE_API ATank : public APawn
{
	GENERATED_BODY()

public:
	//Call by the engine when actor damage is dealt
	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser) override;	

	//return current health as a percentage of starting health
	UFUNCTION(BlueprintPure, Category = "Health")
	float GetHealthPercent() const;

	UFUNCTION(BlueprintPure, Category = "Health")
	float GetCurrentHealth() const;

private:
	// Sets default values for this pawn's properties
	ATank();
	
	UPROPERTY(EditDefaultsOnly, Category="Setup")
	int32 StartingHealth = 100;
	
	UPROPERTY(VisibleAnywhere, Category="Health")
	int32 CurrentHealth = 0; // 在建構函式裡初始化為 StartingHealth
};
