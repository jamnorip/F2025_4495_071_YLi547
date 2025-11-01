// Fill out your copyright notice in the Description page of Project Settings.

#include "PC_Tank.h"
#include "Tank.h" 

void APC_Tank::BeginPlay()
{
	Super::BeginPlay();
	
	// 检查 Pawn 是否存在
	APawn* MyPawn = GetPawn();
	if (!MyPawn)
	{
		UE_LOG(LogTemp, Warning, TEXT("PC_Tank: No pawn possessed yet."));
		return;
	}
}

void APC_Tank::SetPawn(APawn* InPawn)
{
	Super::SetPawn(InPawn);
	if (InPawn)
	{
		auto PossessedTank = Cast<ATank>(InPawn);
		if (!ensure(PossessedTank)) return;

		//subscribe our local method to the tank's death event
		PossessedTank->OnDeath.AddUniqueDynamic(this, &APC_Tank::OnPossessedTankDeath);
	}
}

void APC_Tank::OnPossessedTankDeath()
{
	StartSpectatingOnly();
}