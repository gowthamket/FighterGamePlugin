// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "FighterGamePluginCharacter.h"
#include "GameFramework/Character.h"
#include "YBotCharacter.generated.h"

/**
 * 
 */
UCLASS()
class FIGHTERGAMEPLUGIN_API AYBotCharacter : public AFighterGamePluginCharacter
{
	GENERATED_BODY()
	
		virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent);

};
