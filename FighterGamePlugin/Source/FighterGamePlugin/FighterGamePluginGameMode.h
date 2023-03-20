// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "FighterGamePluginCharacter.h"
#include "FighterGamePluginGameMode.generated.h"

UCLASS(minimalapi)
class AFighterGamePluginGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	AFighterGamePluginGameMode();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player References")
	AFighterGamePluginCharacter* player1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player References")
	AFighterGamePluginCharacter* player2;
};



