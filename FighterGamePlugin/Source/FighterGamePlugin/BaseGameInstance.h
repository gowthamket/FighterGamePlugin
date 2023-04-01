// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "BaseGameInstance.generated.h"

/**
 * 
 */
UENUM(BlueprintType)
enum class ECharacterClass : uint8
{
	VE_Default	UMETA(DisplayName = "Mannequin"),
	VE_YBot		UMETA(DisplayName = "YBot")
};

UCLASS()
class FIGHTERGAMEPLUGIN_API UBaseGameInstance : public UGameInstance
{
	GENERATED_BODY()

public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player References")
		ECharacterClass characterClass;

	//Is the device intended to be used for multiple players (keyboard mode)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Controller")
		bool isDeviceForMultiplePlayers;
};
