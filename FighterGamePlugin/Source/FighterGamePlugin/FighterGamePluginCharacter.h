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

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
		bool hasUsedCommand;
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
	void StartExceptionalAttack();

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
		void P2KeyboardExceptionalAttack();

	UFUNCTION(BlueprintCallable)
		void P2KeyboardJump();

	UFUNCTION(BlueprintCallable)
		void P2KeyboardStopJumping();

	UFUNCTION(BlueprintCallable)
		void P2KeyboardMoveRight(float _value);

protected:

	/** Called for side to side input using keyboard */
	void MoveRight(float Val);

	/** Called for side to side input using gamepad */
	void MoveRightController(float _val);

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

	//Adds input to the input buffer
	UFUNCTION(BlueprintCallable)
		void AddInputToInputBuffer(FInputInfo _inputInfo);

	//Check if the input buffer contains any sequences from the character's list of commands
	UFUNCTION(BlueprintCallable)
		void CheckInputBufferForCommand();

	//Make the character begin using a command based off the command's name
	UFUNCTION(BlueprintCallable)
		void StartCommand(FString _commandName);

	//Make the character stop crouching
	UFUNCTION(BlueprintCallable)
		void StopCrouching();

	//Make the character start blocking
	UFUNCTION(BlueprintCallable)
		void StartBlocking();

	//Make the character stop blocking
	UFUNCTION(BlueprintCallable)
		void StopBlocking();

	//Determine what the character should do when colliding with a proximity hitbox
	UFUNCTION(BlueprintCallable)
		void CollidedWithProximityHitbox();

	//Damage the player
	UFUNCTION(BlueprintCallable)
		void TakeDamage(float _damageAmount, float _hitstunTime, float _blockstunTime);

	UFUNCTION(BlueprintImplementableEvent)
		void AddInputIconToScreen(int _iconIndex, bool _shouldAddInput = true);
	

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

	//The amount of time the character will be stunned (hitStun, blockStun, or from a stunning attack)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
		float stunTime;

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

	//Commands to be used when a correct series of inputs has been pressed
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
		TArray<FCommand> characterCommands;

	//Commands to be used when a correct series of inputs has been pressed
	FCommand tempCommand;

	//Have the temp commands been used
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
		bool hasUsedTempCommand;

	//The array of inputs the player controlling this character has performed
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
		TArray<FInputInfo> inputBuffer;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input Stack")
		bool hasReleasedAxisInput;

	//Has the player used the light exceptional attack
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attacks")
		bool wasLightExAttackUsed;

	//Has the player used the medium exceptional attack
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attacks")
		bool wasMediumExAttackUsed;

	//Has the player used the heavy exceptional attack
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attacks")
		bool wasHeavyExAttackUsed;

	//Has player used the super attack
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attacks")
		bool wasSuperUsed;

	//The amount of super meter the player has
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attacks")
		float superMeterAmount;

public:
	AFighterGamePluginCharacter();

	/** Returns SideViewCameraComponent subobject **/
	FORCEINLINE class UCameraComponent* GetSideViewCameraComponent() const { return SideViewCameraComponent; }
	/** Returns CameraBoom subobject **/
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
};
