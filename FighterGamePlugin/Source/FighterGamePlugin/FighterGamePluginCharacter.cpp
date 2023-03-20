// Copyright Epic Games, Inc. All Rights Reserved.

#include "FighterGamePluginCharacter.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include <FighterGamePlugin/FighterGamePluginGameMode.h>

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
}

//////////////////////////////////////////////////////////////////////////
// Input

void AFighterGamePluginCharacter::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	if (auto gamemode = Cast<AFighterGamePluginGameMode>(GetWorld()->GetAuthGameMode()))
	{
		if (gamemode->player1 == this)
		{
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
		}
		else
		{
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
	
}

void AFighterGamePluginCharacter::MoveRight(float Value)
{
	if (canMove && characterState != ECharacterState::VE_Crouching && characterState != ECharacterState::VE_Blocking)
	{
		if (Value > 0.20f)
		{
			characterState = ECharacterState::VE_MovingRight;
		}
		else if (Value < -0.20f)
		{
			characterState = ECharacterState::VE_MovingLeft;
		}
		else
		{
			characterState = ECharacterState::VE_MovingRight;
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
}

void AFighterGamePluginCharacter::StartAttack2()
{
	wasMediumAttackUsed = true;
}

void AFighterGamePluginCharacter::StartAttack3()
{
	wasHeavyAttackUsed = true;
}

void AFighterGamePluginCharacter::StartAttack4()
{
	wasSpecialAttackUsed = true;
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
		

		if (stunTime > 0.0f)
		{
			characterState = ECharacterState::VE_Stunned;
			BeginStun();
		}

		if (otherPlayer)
		{
			otherPlayer->hasLandedHit = true;
		}
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
			characterState = ECharacterState::VE_Default;
		}
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
	characterState = ECharacterState::VE_Jumping;
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

