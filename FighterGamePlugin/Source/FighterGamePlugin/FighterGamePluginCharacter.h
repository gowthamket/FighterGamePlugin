// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "FighterGamePluginCharacter.generated.h"

UENUM(BlueprintType)
enum class ECharacterState : uint8
{
	VE_Default		UMETA(DisplayName = "NOT_MOVING"),
	VE_MovingRight	UMETA(DisplayName = "MOVING_RIGHT"),
	VE_MovingLeft	UMETA(DisplayName = "MOVING_LEFT"),
	VE_Jumping		UMETA(DisplayName = "JUMPING"),
	VE_Stunned		UMETA(DisplayName = "STUNNED"),
	VE_Blocking		UMETA(DisplayName = "BLOCKING"),
	VE_Crouching	UMETA(DisplayName = "CROUCHING"),
	VE_Launched		UMETA(DisplayName = "LAUNCHED")

};

USTRUCT(BlueprintType)
struct FCommand
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
		FString name;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
		TArray<FString> inputs;
};

USTRUCT(BlueprintType)
struct FInputInfo
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
		FString inputName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
		float timeStamp;
};

UCLASS(config=Game)
class AFighterGamePluginCharacter : public ACharacter
{
	GENERATED_BODY()

	/** Side view camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* SideViewCameraComponent;

	/** Camera boom positioning the camera beside the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class USpringArmComponent* CameraBoom;

	void StartAttack1();
	void StartAttack2();
	void StartAttack3();
	void StartAttack4();
	void StartExcpetionalAttack();

	//When in Keyboard-Only mode, use these functions to perform actions with player 2
	UFUNCTION(BlueprintCallable)
		void P2KeyboardAttack1();

	UFUNCTION(BlueprintCallable)
		void P2KeyboardAttack2();

	UFUNCTION(BlueprintCallable)
		void P2KeyboardAttack3();

	UFUNCTION(BlueprintCallable)
		void P2KeyboardAttack4();

	UFUNCTION(BlueprintCallable)
		void P2KeyboardJump();

	UFUNCTION(BlueprintCallable)
		void P2KeyboardStopJumping();

	UFUNCTION(BlueprintCallable)
		void P2KeyboardMoveRight(float _value);

	UFUNCTION(BlueprintCallable)
		void P2KeyboardExceptionalAttack();

protected:

	/** Called for side to side input */
	void MoveRight(float Val);

	void Tick(float DeltaTime);

	/** Handle touch inputs. */
	void TouchStarted(const ETouchIndex::Type FingerIndex, const FVector Location);

	/** Handle touch stop event. */
	void TouchStopped(const ETouchIndex::Type FingerIndex, const FVector Location);

	// APawn interface
	virtual void SetupPlayerInputComponent(class UInputComponent* InputComponent) override;
	// End of APawn interface

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hitbox")
		AActor* hurtbox;

	//Override the ACharacter and APawn functionality to have functionality to have more control over jumps and landings
	virtual void Jump() override;
	virtual void StopJumping() override;
	virtual void Landed(const FHitResult& Hit) override;

	//Make the character begin crouching
	UFUNCTION(BlueprintCallable)
		void StartCrouching();

	//Enter the stun-state
	void BeginStun();

	//Exit the stun-state
	void ExitStun();

	//Adds inputs to the input buffer
	UFUNCTION(BlueprintCallable)
		void AddInputToInputBuffer(FInputInfo _inputInfo);

	//Removes inputs from the input buffer
	UFUNCTION(BlueprintCallable)
		void RemoveInputFromInputBuffer();

	//Make the character stop crouching
	UFUNCTION(BlueprintCallable)
		void StopCrouching();

	//Make the character start blocking
	UFUNCTION(BlueprintCallable)
		void StartBlocking();

	//Make the character stop blocking
	UFUNCTION(BlueprintCallable)
		void StopBlocking();

	//Determine how far the characters should be pushed back
	UFUNCTION(BlueprintCallable)
	void PerformPushback(float _pushbackAmount, float _launchAmount, bool _hasBlocked);

	//Determine what the character should do when colliding with a proximity hitbox
	UFUNCTION(BlueprintCallable)
		void CollidedWithProximityHitbox();

	//Damage the player
	UFUNCTION(BlueprintCallable)
		void TakeDamage(float _damageAmount, float _hitstunTime, float _blockstunTime, float _pushbackAmount, float _launchAmount);

	UFUNCTION(BlueprintImplementableEvent)
		void ChangeToDamagedMaterials();

	UFUNCTION(BlueprintCallable)
		void StartExceptionalAttack();

	//Check if input buffer contains any sequences from the character's list of commands
	UFUNCTION(BlueprintCallable)
		void CheckInputBufferForCommand();

	//Make the character begin using a command based off of the command's name 
	UFUNCTION(BlueprintCallable)
		void StartCommand(FString _commandName);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player References")
		AFighterGamePluginCharacter* otherPlayer;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Health")
		float playerHealth;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attacks")
		bool wasFirstAttackUsed;

	//The direction the character is moving OR the direction the player is holding down
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
		ECharacterState characterState;

	//The character's transform 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Model")
		FTransform transform;

	//The character's scale 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Model")
		FVector scale;

	//The timer handle for all stuns (hitstuns, blockstuns, and stunning attacks)
		FTimerHandle stunTimerHandle;

	//The timer handle to remove inputs from the input buffer
		FTimerHandle inputBufferTimerHandle;

	//The amount of time the character will be stunned (hitStun, blockStun, or from a stunning attack)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
		float stunTime;

	//The amount of super meter the player has
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Super Meter")
		float superMeterAmount;

	//Has the player used the light attack
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attacks")
	bool wasLightAttackUsed;

	//Has the player used the medium attack
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attacks")
		bool wasMediumAttackUsed;

	//Has the player used the heavy attack
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attacks")
		bool wasHeavyAttackUsed;

	//Has the player used the special attack
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attacks")
		bool wasSpecialAttackUsed;

	//Has the player used the super attack?
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attacks")
		bool wasSuperUsed;

	//Has the player used the light exceptional attack
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attacks")
		bool wasLightExAttackUsed;

	//Has the player used the medium exceptional attack
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attacks")
		bool wasMediumExAttackUsed;

	//Has the player used the heavy exceptional attack
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attacks")
		bool wasHeavyExAttackUsed;

	//Can the player use exceptional attacks?
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attacks")
		bool canUseExAttack;

	//Is the character's model flipped
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Model")
		bool isFlipped;

	//Has the player landed a hit with their last attack
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attacks")
		bool hasLandedHit;

	//Is the player able to currently able to move?
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attacks")
		bool canMove;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Controller")
		bool isDeviceForMultiplePlayers;

	//The maximum amount of distance that the players can be apart
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Health")
		float maxDistanceApart;

	//The scaled value of gravity (determines how long players will stay in the air)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
		float gravityScale;

	//The amount of time before inputs are removed from the input buffer
	float removeInputFromBufferTime;

	//The array of inputs the player controlling this character has performed
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
		TArray<FInputInfo> inputBuffer;

	//Commands to be used when a correct series of inputs has been pressed
	FCommand tempCommand;

	//Has temp command been used
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Commands")
		bool hasUsedTempCommand;

public:
	AFighterGamePluginCharacter();

	/** Returns SideViewCameraComponent subobject **/
	FORCEINLINE class UCameraComponent* GetSideViewCameraComponent() const { return SideViewCameraComponent; }
	/** Returns CameraBoom subobject **/
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
};
