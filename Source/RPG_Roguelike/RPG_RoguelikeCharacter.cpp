// Copyright Epic Games, Inc. All Rights Reserved.

#include "RPG_RoguelikeCharacter.h"
#include "HeadMountedDisplayFunctionLibrary.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/SpringArmComponent.h"

//////////////////////////////////////////////////////////////////////////
// ARPG_RoguelikeCharacter

ARPG_RoguelikeCharacter::ARPG_RoguelikeCharacter()
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

	// set our turn rates for input
	BaseTurnRate = 45.f;
	BaseLookUpRate = 45.f;

	// Don't rotate when the controller rotates. Let that just affect the camera.
	//bUseControllerRotationPitch = false;
	//bUseControllerRotationYaw = false;
	//bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true; // Character moves in the direction of input...	
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 540.0f, 0.0f); // ...at this rotation rate
	GetCharacterMovement()->JumpZVelocity = 600.f;
	GetCharacterMovement()->AirControl = 0.2f;

	// These keep the character always facing forward
	GetCharacterMovement()->bOrientRotationToMovement = false;
	GetCharacterMovement()->bUseControllerDesiredRotation = true;


	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 300.0f; // The camera follows at this distance behind the character	
	CameraBoom->bUsePawnControlRotation = true; // Rotate the arm based on the controller

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	FollowCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm

	// Note: The skeletal mesh and anim blueprint references on the Mesh component (inherited from Character) 
	// are set in the derived blueprint asset named MyCharacter (to avoid direct content references in C++)
	attack1;
}

//////////////////////////////////////////////////////////////////////////
// Input

void ARPG_RoguelikeCharacter::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	// Set up gameplay key bindings
	check(PlayerInputComponent);
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);

	PlayerInputComponent->BindAction("Sprint", IE_Pressed, this, &ARPG_RoguelikeCharacter::Sprint);
	PlayerInputComponent->BindAction("Sprint", IE_Released, this, &ARPG_RoguelikeCharacter::StopSprinting);

	PlayerInputComponent->BindAxis("MoveForward", this, &ARPG_RoguelikeCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &ARPG_RoguelikeCharacter::MoveRight);


	// These are the bindings for making attacks
	PlayerInputComponent->BindAction("LightAttack", IE_Pressed, this, &ARPG_RoguelikeCharacter::AttackLight);
	PlayerInputComponent->BindAction("HeavyAttack", IE_Pressed, this, &ARPG_RoguelikeCharacter::AttackHeavy);

	// We have 2 versions of the rotation bindings to handle different kinds of devices differently
	// "turn" handles devices that provide an absolute delta, such as a mouse.
	// "turnrate" is for devices that we choose to treat as a rate of change, such as an analog joystick
	PlayerInputComponent->BindAxis("Turn", this, &APawn::AddControllerYawInput);
	PlayerInputComponent->BindAxis("TurnRate", this, &ARPG_RoguelikeCharacter::TurnAtRate);
	PlayerInputComponent->BindAxis("LookUp", this, &APawn::AddControllerPitchInput);
	PlayerInputComponent->BindAxis("LookUpRate", this, &ARPG_RoguelikeCharacter::LookUpAtRate);

	// handle touch devices
	PlayerInputComponent->BindTouch(IE_Pressed, this, &ARPG_RoguelikeCharacter::TouchStarted);
	PlayerInputComponent->BindTouch(IE_Released, this, &ARPG_RoguelikeCharacter::TouchStopped);

	// VR headset functionality
	PlayerInputComponent->BindAction("ResetVR", IE_Pressed, this, &ARPG_RoguelikeCharacter::OnResetVR);
	maxHealth = 100.0f;
	currentHealth = maxHealth;

	maxMana = 100.0f;
	currentMana = maxMana;
	isAttacking = false;
	isStunned = false;
}


void ARPG_RoguelikeCharacter::OnResetVR()
{
	// If RPG_Roguelike is added to a project via 'Add Feature' in the Unreal Editor the dependency on HeadMountedDisplay in RPG_Roguelike.Build.cs is not automatically propagated
	// and a linker error will result.
	// You will need to either:
	//		Add "HeadMountedDisplay" to [YourProject].Build.cs PublicDependencyModuleNames in order to build successfully (appropriate if supporting VR).
	// or:
	//		Comment or delete the call to ResetOrientationAndPosition below (appropriate if not supporting VR)
	UHeadMountedDisplayFunctionLibrary::ResetOrientationAndPosition();
}

void ARPG_RoguelikeCharacter::TouchStarted(ETouchIndex::Type FingerIndex, FVector Location)
{
		Jump();
}

void ARPG_RoguelikeCharacter::TouchStopped(ETouchIndex::Type FingerIndex, FVector Location)
{
		StopJumping();
}

void ARPG_RoguelikeCharacter::TurnAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerYawInput(Rate * BaseTurnRate * GetWorld()->GetDeltaSeconds());
}

void ARPG_RoguelikeCharacter::LookUpAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerPitchInput(Rate * BaseLookUpRate * GetWorld()->GetDeltaSeconds());
}

void ARPG_RoguelikeCharacter::MoveForward(float Value)
{
	if ((Controller != nullptr) && (Value != 0.0f) && CanMove())
	{
		// find out which way is forward
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get forward vector
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		AddMovementInput(Direction, Value);
	}
}

void ARPG_RoguelikeCharacter::MoveRight(float Value)
{
	if ( (Controller != nullptr) && (Value != 0.0f) && CanMove())
	{
		// find out which way is right
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);
	
		// get right vector 
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
		// add movement in that direction
		AddMovementInput(Direction, Value);
	}
}


void ARPG_RoguelikeCharacter::Sprint()
{
	GetCharacterMovement()->MaxWalkSpeed = 900.f;
}

void ARPG_RoguelikeCharacter::StopSprinting()
{
	GetCharacterMovement()->MaxWalkSpeed = 600.f;
}

void ARPG_RoguelikeCharacter::TakeDamage(float damageAmount)
{
	currentHealth -= damageAmount;

	if (currentHealth < 0.0)
	{
		currentHealth = 0.0f;
	}
}

void ARPG_RoguelikeCharacter::TakeDamagePercent(float damagePercent)
{
	currentHealth -= damagePercent * maxHealth;

	if (currentHealth < 0.0)
	{
		currentHealth = 0.0f;
	}
	
}

void ARPG_RoguelikeCharacter::HealAmount(float healAmount)
{
	currentHealth += healAmount;

	if (currentHealth > maxHealth)
	{
		currentHealth = maxHealth;
	}
}

void ARPG_RoguelikeCharacter::HealPercent(float healPercent)
{
	currentHealth += healPercent * maxHealth;

	if (currentHealth > maxHealth)
	{
		currentHealth = maxHealth;
	}
}

void ARPG_RoguelikeCharacter::AttackLight()
{
	//UE_LOG(LogTemp, Warning, TEXT("light attack"));
	if (!isAttacking)
	{
		attackType = 0;
		isAttacking = true;
	}
}

void ARPG_RoguelikeCharacter::AttackHeavy()
{
	if (!isAttacking)
	{
		attackType = 1;
		isAttacking = true;
	}
}

void ARPG_RoguelikeCharacter::AttackCombo()
{
	if (!isAttacking)
	{
		attackType = 2;
	}
}

void ARPG_RoguelikeCharacter::EndAttack()
{
	isAttacking = false;
}

bool ARPG_RoguelikeCharacter::CanMove()
{
	return (!isAttacking) && (!isStunned);
}