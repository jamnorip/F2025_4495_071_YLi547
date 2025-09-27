// Copyright Epic Games, Inc. All Rights Reserved.

#include "WorldRebalanceGameMode.h"
#include "WorldRebalanceCharacter.h"
#include "UObject/ConstructorHelpers.h"

AWorldRebalanceGameMode::AWorldRebalanceGameMode()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/ThirdPerson/Blueprints/BP_ThirdPersonCharacter"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
}
