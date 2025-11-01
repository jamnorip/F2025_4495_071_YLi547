// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "PC_Tank.generated.h"

class ATank;


/**
 * 
 */
UCLASS()
class WORLDREBALANCE_API APC_Tank : public APlayerController
{
	GENERATED_BODY()

public:
	virtual void BeginPlay() override;
	
	void SetPawn(APawn* InPawn);
	
	UFUNCTION()
	void OnPossessedTankDeath();
};
