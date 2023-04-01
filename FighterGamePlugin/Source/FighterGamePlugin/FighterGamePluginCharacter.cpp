// Copyright Epic Games, Inc. All Rights Reserved.

#include "FighterGamePluginCharacter.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include <FighterGamePlugin/FighterGamePluginGameMode.h>
#include <FighterGamePlugin/BaseGameInstance.h>

AFighterGamePluginCharacter::AFighterGamePluginCharacter()
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

	// Don't rotate when the controller rotates.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Create a camera boom attached to the root (capsule)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->SetUsingAbsoluteRotation(true); // Rotation of the character should not affect rotation of boom
	CameraBoom->bDoCollisionTest = false;
	CameraBoom->TargetArmLength = 500.f;
	CameraBoom->SocketOffset = FVector(0.f,0.f,75.f);
	CameraBoom->SetRelativeRotation(FRotator(0.f,180.f,0.f));

	// Create a camera and attach to boom
	SideViewCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("SideViewCamera"));
	SideViewCameraComponent->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	SideViewCameraComponent->bUsePawnControlRotation = false; // We don't want the controller rotating the camera

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true; // Face in the direction we are moving..
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 720.0f, 0.0f); // ...at this rotation rate
	GetCharacterMovement()->GravityScale = 2.f;
	GetCharacterMovement()->AirControl = 0.80f;
	GetCharacterMovement()->JumpZVelocity = 1000.f;
	GetCharacterMovement()->GroundFriction = 3.f;
	GetCharacterMovement()->MaxWalkSpeed = 600.f;
	GetCharacterMovement()->MaxFlySpeed = 600.f;
	//GetCharacterMovement()->OrientRotationToMovement = false;

	// Note: The skeletal mesh and anim blueprint references on the Mesh component (inherited from Character) 
	// are set in the derived blueprint asset named MyCharacter (to avoid direct content references in C++)

	wasFirstAttackUsed = false;
	playerHealth = 1.00f;
	characterState = ECharacterState::VE_Default;
	transform = FTransform();
	scale = FVector(0.0f, 0.0f, 0.0f);
	playerHealth = 1.00f;
	wasLightAttackUsed = false;
	wasMediumAttackUsed = false;
	wasHeavyAttackUsed = false;
	wasSpecialAttackUsed = false;
	isFlipped = false;
	hasLandedHit = false;
	canMove = false;
	maxDistanceApart = 800.0f;
	stunTime = 0.0f;
	hurtbox = nullptr;
	hasUsedTempCommand = false;
	wasLightExAttackUsed = false;
	wasHeavyExAttackUsed = false;
	wasMediumExAttackUsed = false;
	wasSuperUsed = false;
	superMeterAmount = 0.0f;


	hasReleasedAxisInput = true;

	//Create and assign the character's commands
	characterCommands.SetNum(2);

	//Command #1 assignments
	characterCommands[0].name = "Command #1";
	characterCommands[0].inputs.Add("A");
	characterCommands[0].inputs.Add("B");
	characterCommands[0].inputs.Add("C");
	characterCommands[0].hasUsedCommand = false;

	//Command #2 assignments
	characterCommands[1].name = "Command #2";
	characterCommands[1].inputs.Add("A");
	characterCommands[1].inputs.Add("B");
	characterCommands[1].inputs.Add("C");
	characterCommands[1].hasUsedCommand = false;
}

//////////////////////////////////////////////////////////////////////////
// Input

void AFighterGamePluginCharacter::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	if (auto gamemode = Cast<AFighterGamePluginGameMode>(GetWorld()->GetAuthGameMode()))
	{
		if (gamemode->player1 == this)
		{
			PlayerInputComponent->BindAxis("MoveRightP1", this, &AFighterGamePluginCharacter::MoveRight);
		}
		else
		{
			PlayerInputComponent->BindAxis("MoveRightP2", this, &AFighterGamePluginCharacter::MoveRight);
		}

		// set up gameplay key bindings
		PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &AFighterGamePluginCharacter::Jump);
		PlayerInputComponent->BindAction("Jump", IE_Released, this, &AFighterGamePluginCharacter::StopJumping);
		PlayerInputComponent->BindAction("Crouch", IE_Pressed, this, &AFighterGamePluginCharacter::StartCrouching);
		PlayerInputComponent->BindAction("Crouch", IE_Released, this, &AFighterGamePluginCharacter::StopCrouching);
		PlayerInputComponent->BindAction("Block", IE_Pressed, this, &AFighterGamePluginCharacter::StartBlocking);
		PlayerInputComponent->BindAction("Block", IE_Released, this, &AFighterGamePluginCharacter::StopBlocking);
		PlayerInputComponent->BindAxis("MoveRight", this, &AFighterGamePluginCharacter::MoveRight);

		PlayerInputComponent->BindAction("Attack1P1", IE_Pressed, this, &AFighterGamePluginCharacter::StartAttack1);
		PlayerInputComponent->BindAction("Attack2P2", IE_Pressed, this, &AFighterGamePluginCharacter::StartAttack2);
		PlayerInputComponent->BindAction("Attack3P3", IE_Pressed, this, &AFighterGamePluginCharacter::StartAttack3);
		PlayerInputComponent->BindAction("Attack4P4", IE_Pressed, this, &AFighterGamePluginCharacter::StartAttack4);

		PlayerInputComponent->BindTouch(IE_Pressed, this, &AFighterGamePluginCharacter::TouchStarted);
		PlayerInputComponent->BindTouch(IE_Released, this, &AFighterGamePluginCharacter::TouchStopped);

		// set up gameplay key bindings
		PlayerInputComponent->BindAction("JumpP2", IE_Pressed, this, &AFighterGamePluginCharacter::Jump);
		PlayerInputComponent->BindAction("JumpP2", IE_Released, this, &AFighterGamePluginCharacter::StopJumping);
		PlayerInputComponent->BindAction("CrouchP2", IE_Pressed, this, &AFighterGamePluginCharacter::StartCrouching);
		PlayerInputComponent->BindAction("CrouchP2", IE_Released, this, &AFighterGamePluginCharacter::StopCrouching);
		PlayerInputComponent->BindAction("BlockP2", IE_Pressed, this, &AFighterGamePluginCharacter::StartBlocking);
		PlayerInputComponent->BindAction("BlockP2", IE_Released, this, &AFighterGamePluginCharacter::StopBlocking);
		PlayerInputComponent->BindAxis("MoveRightP2", this, &AFighterGamePluginCharacter::MoveRight);

		PlayerInputComponent->BindAction("Attack1P2", IE_Pressed, this, &AFighterGamePluginCharacter::StartAttack1);
		PlayerInputComponent->BindAction("Attack2P2", IE_Pressed, this, &AFighterGamePluginCharacter::StartAttack2);
		PlayerInputComponent->BindAction("Attack3P2", IE_Pressed, this, &AFighterGamePluginCharacter::StartAttack3);
		PlayerInputComponent->BindAction("Attack4P2", IE_Pressed, this, &AFighterGamePluginCharacter::StartAttack4);

		PlayerInputComponent->BindTouch(IE_Pressed, this, &AFighterGamePluginCharacter::TouchStarted);
		PlayerInputComponent->BindTouch(IE_Released, this, &AFighterGamePluginCharacter::TouchStopped);
	}
	
}

void AFighterGamePluginCharacter::MoveRight(float Value)
{
	if (auto baseGameInstance = Cast<UBaseGameInstance>(GetGameInstance()))
	{
		if (baseGameInstance->isDeviceForMultiplePlayers)
		{
			if (canMove && characterState != ECharacterState::VE_Crouching && characterState != ECharacterState::VE_Blocking)
			{
				if (characterState != ECharacterState::VE_Jumping && characterState != ECharacterState::VE_Launched)
				{
					if (Value > 0.20f)
					{
						characterState = ECharacterState::VE_MovingRight;
						AddInputIconToScreen(1, hasReleasedAxisInput);
						hasReleasedAxisInput = false;
					}
					else if (Value < -0.20f)
					{
						characterState = ECharacterState::VE_MovingLeft;
						AddInputIconToScreen(3, hasReleasedAxisInput);
						hasReleasedAxisInput = false;
					}
					else
					{
						characterState = ECharacterState::VE_MovingRight;
						hasReleasedAxisInput = true;
					}
				}
			}

			float currentDistanceApart = abs(otherPlayer->GetActorLocation().Y - GetActorLocation().Y);

			if (currentDistanceApart >= maxDistanceApart)
			{
				if ((currentDistanceApart + Value < currentDistanceApart && !isFlipped) || (currentDistanceApart - Value < currentDistanceApart && isFlipped))
				{
					AddMovementInput(FVector(0.f, 1.0f, 0.f), Value);
				}
				else
				{
					AddMovementInput(FVector(0.0f, 1.0f, 0.0f), Value);
				}
			}

		}
	}
	
		
}

void AFighterGamePluginCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (characterState != ECharacterState::VE_Jumping)
	{
		if (otherPlayer)
		{
			if (auto characterMovement = GetCharacterMovement())
			{
				if (auto enemyMovement = otherPlayer->GetCharacterMovement())
				{
					if (enemyMovement->GetActorLocation().Y <= characterMovement->GetActorLocation().Y)
					{
						if (isFlipped)
						{
							if (auto mesh = GetCapsuleComponent()->GetChildComponent(1))
							{
								transform = mesh->GetRelativeTransform();
								scale = transform.GetScale3D();
								scale.Y = -1;
								transform.SetScale3D(scale);
								mesh->SetRelativeTransform(transform);
							}
							isFlipped = false;
						}
					}
					else
					{
						if (!isFlipped)
						{
							if (auto mesh = GetCapsuleComponent()->GetChildComponent(1))
							{
								transform = mesh->GetRelativeTransform();
								scale = transform.GetScale3D();
								scale.Y = -1;
								transform.SetScale3D(scale);
								mesh->SetRelativeTransform(transform);
							}
							isFlipped = true;
						}
					}
				}
			}
		}
	}
}

void AFighterGamePluginCharacter::TouchStarted(const ETouchIndex::Type FingerIndex, const FVector Location)
{
	// jump on any touch
	Jump();
}

void AFighterGamePluginCharacter::TouchStopped(const ETouchIndex::Type FingerIndex, const FVector Location)
{
	StopJumping();
}

void AFighterGamePluginCharacter::StartAttack1()
{
	wasLightAttackUsed = true;
	AddInputIconToScreen(4);
}

void AFighterGamePluginCharacter::StartAttack2()
{
	wasMediumAttackUsed = true;
	AddInputIconToScreen(5);
}

void AFighterGamePluginCharacter::StartAttack3()
{
	wasHeavyAttackUsed = true;
	AddInputIconToScreen(6);
}

void AFighterGamePluginCharacter::StartAttack4()
{
	if (superMeterAmount >= 1.0f)
	{
		wasSuperUsed = true;
		AddInputIconToScreen(7);
	}
	
}

void AFighterGamePluginCharacter::StartExceptionalAttack()
{
	if (wasLightAttackUsed)
	{
		wasLightExAttackUsed = true;
		superMeterAmount -= 0.20f;
	}
	else if (wasMediumAttackUsed)
	{
		wasMediumExAttackUsed = true;
		superMeterAmount -= 0.35f;
	}
	else if (wasHeavyAttackUsed)
	{
		wasHeavyExAttackUsed = true;
		superMeterAmount -= 0.50f;
	}

	if (superMeterAmount < 0.00f)
	{
		superMeterAmount = 0.00f;
	}
}

void AFighterGamePluginCharacter::CollidedWithProximityHitbox()
{
	if ((characterState == ECharacterState::VE_MovingLeft && !isFlipped) || (characterState == ECharacterState::VE_MovingRight && isFlipped))
	{
		characterState = ECharacterState::VE_Blocking;
	}
}

void AFighterGamePluginCharacter::TakeDamage(float _damageAmount, float _hitstunTime, float _blockstunTime)
{
	if (characterState != ECharacterState::VE_Blocking)
	{
		
		stunTime = _hitstunTime;
		playerHealth -= _damageAmount;
		superMeterAmount += _damageAmount * 0.85f;

		if (stunTime > 0.0f)
		{
			characterState = ECharacterState::VE_Stunned;
			BeginStun();
		}

		if (otherPlayer)
		{
			otherPlayer->hasLandedHit = true;
			//otherPlayer->PerformPushback(_pushbackAmount, 0.0f, false);

			if (!otherPlayer->wasLightExAttackUsed)
			{
				otherPlayer->superMeterAmount += _damageAmount * 0.30f;
			}
		}

		//PerformPushback(_pushbackAmount, _launchAmount, false);
	}
	else
	{
		float reducedDamage = _damageAmount * 0.5f;
		playerHealth -= reducedDamage;

		stunTime = _blockstunTime;

		if (stunTime > 0.0f)
		{
			BeginStun();
		}
		else
		{
			if (characterState != ECharacterState::VE_Launched)
			{
				characterState = ECharacterState::VE_Default;
			}
			
		}

		if (otherPlayer)
		{
			otherPlayer->hasLandedHit = false;
			//otherPlayer->PerformPushback(_pushbackAmount, 0.0f, false);
		}

		//PerformPushback(_pushbackAmount, 0.0f, true);
	}
	
	

	if (playerHealth < 0.00f)
	{
		playerHealth = 0.00f;
	}
}

void AFighterGamePluginCharacter::P2KeyboardAttack1()
{
	StartAttack1();
}

void AFighterGamePluginCharacter::P2KeyboardAttack2()
{
	StartAttack2();
}

void AFighterGamePluginCharacter::P2KeyboardAttack3()
{
	StartAttack3();
}

void AFighterGamePluginCharacter::P2KeyboardAttack4()
{
	StartAttack4();
}

void AFighterGamePluginCharacter::P2KeyboardExceptionalAttack()
{
	StartExceptionalAttack();
}

void AFighterGamePluginCharacter::P2KeyboardJump()
{
	Jump();
}

void AFighterGamePluginCharacter::P2KeyboardStopJumping()
{
	StopJumping();
}

void AFighterGamePluginCharacter::P2KeyboardMoveRight(float _value)
{
	MoveRight(_value);
}

void AFighterGamePluginCharacter::Jump()
{
	if (canMove)
	{
		ACharacter::Jump();
		characterState = ECharacterState::VE_Jumping;
		AddInputIconToScreen(0);
	}
	
}

void AFighterGamePluginCharacter::StopJumping()
{
	characterState = ECharacterState::VE_Default;
}

void AFighterGamePluginCharacter::Landed(const FHitResult& Hit)
{
	characterState = ECharacterState::VE_Default;
}

void AFighterGamePluginCharacter::StartCrouching()
{
	characterState = ECharacterState::VE_Crouching;
}

void AFighterGamePluginCharacter::StopCrouching()
{
	characterState = ECharacterState::VE_Default;
}

void AFighterGamePluginCharacter::StartBlocking()
{
	characterState = ECharacterState::VE_Blocking;
}

void AFighterGamePluginCharacter::StopBlocking()
{
	characterState = ECharacterState::VE_Default;
}

void AFighterGamePluginCharacter::BeginStun()
{
	canMove = false;

	GetWorld()->GetTimerManager().SetTimer(stunTimerHandle, this, &AFighterGamePluginCharacter::ExitStun, stunTime, false);
}

void AFighterGamePluginCharacter::ExitStun()
{
	characterState = ECharacterState::VE_Default;
	canMove = true;
}

void AFighterGamePluginCharacter::AddInputToInputBuffer(FInputInfo _inputInfo)
{
	inputBuffer.Add(_inputInfo);
	CheckInputBufferForCommand();
}

void AFighterGamePluginCharacter::CheckInputBufferForCommand()
{
	int correctSequenceCounter = 0;

	for (auto currentCommand : characterCommands)
	{
		for (int commandInput = 0; commandInput < tempCommand.inputs.Num(); ++commandInput)
		{
			for (int input = 0; input < inputBuffer.Num(); ++input)
			{
				if (input + correctSequenceCounter < inputBuffer.Num())
				{
					if (inputBuffer[input + correctSequenceCounter].inputName.Compare(tempCommand.inputs[commandInput]) == 0)
					{
						++correctSequenceCounter;

						if (correctSequenceCounter == tempCommand.inputs.Num())
						{
							StartCommand(tempCommand.name);
						}

						break;
					}
					else
					{
						correctSequenceCounter = 0;
					}
				}
				else
				{
					correctSequenceCounter = 0;
				}
			}
		}
	}

	
}

void AFighterGamePluginCharacter::StartCommand(FString _commandName)
{
	for (int currentCommand = 0; currentCommand < characterCommands.Num(); ++currentCommand)
	{
		if (_commandName.Compare(characterCommands[currentCommand].name) == 0)
		{
			characterCommands[currentCommand].hasUsedCommand = true;
		}
	}
	
}

