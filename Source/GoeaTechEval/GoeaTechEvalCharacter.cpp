// Copyright Epic Games, Inc. All Rights Reserved.

#include "GoeaTechEvalCharacter.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/SpringArmComponent.h"
#include "CustomCharacterMovementComponent.h"

//////////////////////////////////////////////////////////////////////////
// AGoeaTechEvalCharacter

AGoeaTechEvalCharacter::AGoeaTechEvalCharacter(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer.SetDefaultSubobjectClass<UCustomCharacterMovementComponent>(ACharacter::CharacterMovementComponentName))
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

	// set our turn rates for input
	BaseTurnRate = 45.f;
	BaseLookUpRate = 45.f;

	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	MovementComponent = Cast<UCustomCharacterMovementComponent>(GetCharacterMovement());

	// Configure character movement
	GetCustomCharacterMovement()->bOrientRotationToMovement = true; // Character moves in the direction of input...	
	GetCustomCharacterMovement()->RotationRate = FRotator(0.0f, 540.0f, 0.0f); // ...at this rotation rate
	GetCustomCharacterMovement()->JumpZVelocity = 400.0f;
	GetCustomCharacterMovement()->AirControl = 0.2f;

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
	IsAttacking = false;
}

//////////////////////////////////////////////////////////////////////////
// Input

void AGoeaTechEvalCharacter::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	// Set up gameplay key bindings
	check(PlayerInputComponent);
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &AGoeaTechEvalCharacter::Jump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);

	PlayerInputComponent->BindAction("Climb", IE_Pressed, this, &AGoeaTechEvalCharacter::Climb);
	PlayerInputComponent->BindAction("Cancel Climb", IE_Pressed, this, &AGoeaTechEvalCharacter::CancelClimb);

	PlayerInputComponent->BindAxis("MoveForward", this, &AGoeaTechEvalCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &AGoeaTechEvalCharacter::MoveRight);

	// We have 2 versions of the rotation bindings to handle different kinds of devices differently
	// "turn" handles devices that provide an absolute delta, such as a mouse.
	// "turnrate" is for devices that we choose to treat as a rate of change, such as an analog joystick
	PlayerInputComponent->BindAxis("Turn", this, &APawn::AddControllerYawInput);
	PlayerInputComponent->BindAxis("TurnRate", this, &AGoeaTechEvalCharacter::TurnAtRate);
	PlayerInputComponent->BindAxis("LookUp", this, &APawn::AddControllerPitchInput);
	PlayerInputComponent->BindAxis("LookUpRate", this, &AGoeaTechEvalCharacter::LookUpAtRate);

	// handle touch devices
	PlayerInputComponent->BindTouch(IE_Pressed, this, &AGoeaTechEvalCharacter::TouchStarted);
	PlayerInputComponent->BindTouch(IE_Released, this, &AGoeaTechEvalCharacter::TouchStopped);
}

void AGoeaTechEvalCharacter::TouchStarted(ETouchIndex::Type FingerIndex, FVector Location)
{
	Jump();
}

void AGoeaTechEvalCharacter::TouchStopped(ETouchIndex::Type FingerIndex, FVector Location)
{
	StopJumping();
}

void AGoeaTechEvalCharacter::TurnAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerYawInput(Rate * BaseTurnRate * GetWorld()->GetDeltaSeconds());
}

void AGoeaTechEvalCharacter::LookUpAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerPitchInput(Rate * BaseLookUpRate * GetWorld()->GetDeltaSeconds());
}

void AGoeaTechEvalCharacter::Tick(float DeltaTime)
{
	// UE_LOG(LogTemp, Warning, TEXT("Movement type: %d"), MovementComponent->MovementMode);
	Super::Tick(DeltaTime);
}

void AGoeaTechEvalCharacter::MoveForward(float Value)
{
	if (!IsAttacking && (Controller != nullptr) && (Value != 0.0f))
	{
		FVector Direction;
		if (MovementComponent->IsClimbing())
		{
			Direction = FVector::CrossProduct(MovementComponent->GetClimbSurfaceNormal(), -GetActorRightVector());
		}
		else
		{
			// find out which way is forward
			const FRotator Rotation = Controller->GetControlRotation();
			const FRotator YawRotation(0, Rotation.Yaw, 0);

			// get forward vector
			Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);

		}
		AddMovementInput(Direction, Value);
	}
}

void AGoeaTechEvalCharacter::MoveRight(float Value)
{
	if (!IsAttacking && (Controller != nullptr) && (Value != 0.0f))
	{
		FVector Direction;
		if (MovementComponent->IsClimbing())
		{
			Direction = FVector::CrossProduct(MovementComponent->GetClimbSurfaceNormal(), GetActorUpVector());
		}
		else
		{
			// find out which way is right
			const FRotator Rotation = Controller->GetControlRotation();
			const FRotator YawRotation(0, Rotation.Yaw, 0);

			// get right vector 
			Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
		}

		// add movement in that direction
		AddMovementInput(Direction, Value);
	}
}

void AGoeaTechEvalCharacter::Jump()
{
	// Prevent jump when attacking
	if (!IsAttacking) Super::Jump();
}

void AGoeaTechEvalCharacter::Climb()
{
	MovementComponent->TryClimbing();
}

void AGoeaTechEvalCharacter::CancelClimb()
{
	MovementComponent->CancelClimbing();
}
