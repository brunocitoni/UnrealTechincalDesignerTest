// Copyright Epic Games, Inc. All Rights Reserved.

#include "LiithosTestBCGameMode.h"
#include "LiithosTestBCCharacter.h"
#include "UObject/ConstructorHelpers.h"

ALiithosTestBCGameMode::ALiithosTestBCGameMode()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/ThirdPerson/Blueprints/BP_ThirdPersonCharacter"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
}
