// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/BlasterCharacter.h"

#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"

// Sets default values
ABlasterCharacter::ABlasterCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(GetMesh());
	CameraBoom->TargetArmLength = 600.0f;
	CameraBoom->bUsePawnControlRotation = true;

	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false;

	bUseControllerRotationYaw = false;
	GetCharacterMovement()->bOrientRotationToMovement = true;
}

// Called when the game starts or when spawned
void ABlasterCharacter::BeginPlay()
{
	Super::BeginPlay();
	
	if (Controller)
	{
		UE_LOG(LogTemp, Display, TEXT("%s is possessed by %s"), *GetName(), *Controller->GetName());
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("%s is not possessed"), *GetName());
	}

	if (APlayerController* PlayerController = Cast<APlayerController>(Controller))
	{
		UE_LOG(LogTemp, Display, TEXT("Player Controller is %s"), *PlayerController->GetName());
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			if (InputMapping)
			{
				Subsystem->AddMappingContext(InputMapping, 0);
				UE_LOG(LogTemp, Display, TEXT("Input Mapping Context added"));
			}
			else
			{
				UE_LOG(LogTemp, Error, TEXT("InputMapping is null!"));
			}
		}
	}
}

// Called every frame
void ABlasterCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void ABlasterCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (UEnhancedInputComponent* Input = CastChecked<UEnhancedInputComponent>(PlayerInputComponent))
	{
		if (MoveAction)
		{
			UE_LOG(LogTemp, Display, TEXT("Binding MoveAction"));
			Input->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ABlasterCharacter::MoveInput);
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("MoveAction is null!"));
		}

		if (LookAction)
		{
			UE_LOG(LogTemp, Display, TEXT("Binding LookAction"));
			Input->BindAction(LookAction, ETriggerEvent::Triggered, this, &ABlasterCharacter::LookInput);
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("LookAction is null!"));
		}

		if (JumpAction)
		{
			UE_LOG(LogTemp, Display, TEXT("Binding JumpAction"));
			Input->BindAction(JumpAction, ETriggerEvent::Started, this, &ABlasterCharacter::JumpInput);
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("JumpAction is null!"));
		}

		if (JumpAction)
		{
			UE_LOG(LogTemp, Display, TEXT("Binding JumpAction"));
			Input->BindAction(JumpAction, ETriggerEvent::Completed, this, &ABlasterCharacter::StopJumpingInput);
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("JumpAction is null!"));
		}
	}
}

void ABlasterCharacter::MoveInput(const FInputActionInstance& Instance)
{
	if (!Controller)
		return;

	FVector2D MovementDirection = Instance.GetValue().Get<FVector2D>();
	const FRotator Rotation(0.f, Controller->GetControlRotation().Yaw, Controller->GetControlRotation().Roll);
	const FVector RightDirection(FRotationMatrix(Rotation).GetUnitAxis(EAxis::Y));
	const FVector ForwardDirection(FRotationMatrix(Rotation).GetUnitAxis(EAxis::X));
	AddMovementInput(RightDirection, MovementDirection.X);
	AddMovementInput(ForwardDirection, MovementDirection.Y);
}

void ABlasterCharacter::LookInput(const FInputActionInstance& Instance)
{
	FVector2D LookDirection = Instance.GetValue().Get<FVector2D>();
	AddControllerYawInput(LookDirection.X);
	AddControllerPitchInput(LookDirection.Y);
}

void ABlasterCharacter::JumpInput(const FInputActionInstance& Instance)
{
	Super::Jump();
}

void ABlasterCharacter::StopJumpingInput(const FInputActionInstance& Instance)
{
	Super::StopJumping();
}

