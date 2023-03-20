// Copyright Epic Games, Inc. All Rights Reserved.

#include "FighterGamePluginGameMode.h"
#include "FighterGamePluginCharacter.h"
#include "UObject/ConstructorHelpers.h"

AFighterGamePluginGameMode::AFighterGamePluginGameMode()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/SideScrollerCPP/Blueprints/YBotCharacter"));
	if (PlayerPawnBPClass.Class != nullptr)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
}
