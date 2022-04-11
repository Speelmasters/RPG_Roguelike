// Copyright Epic Games, Inc. All Rights Reserved.

#include "RPG_RoguelikeGameMode.h"
#include "RPG_RoguelikeCharacter.h"
#include "UObject/ConstructorHelpers.h"

ARPG_RoguelikeGameMode::ARPG_RoguelikeGameMode()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/ThirdPersonCPP/Blueprints/ThirdPersonCharacter"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
}
