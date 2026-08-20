// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/BlasterCharacter.h"

#include "BlasterComponents/CombatComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/WidgetComponent.h"
#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "HUD/OverheadWidget.h"
#include "Net/UnrealNetwork.h"
#include "Weapon/Weapon.h"

// Sets default values
ABlasterCharacter::ABlasterCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(GetRootComponent());
	CameraBoom->TargetArmLength = 600.0f;
	CameraBoom->bUsePawnControlRotation = true;

	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false;

	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;
	bUseControllerRotationPitch = false;

	GetCharacterMovement()->bOrientRotationToMovement = true;

	OverheadWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("OverheadWidget"));
	OverheadWidget->SetupAttachment(RootComponent);

	CombatComponent = CreateDefaultSubobject<UCombatComponent>(TEXT("CombatComponent"));
	CombatComponent->SetIsReplicated(true);

	GetCharacterMovement()->NavAgentProps.bCanCrouch = true;
}

void ABlasterCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(ABlasterCharacter, OverlappingWeapon, COND_OwnerOnly);
}

void ABlasterCharacter::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	if (CombatComponent)
	{
		CombatComponent->Character = this;
	}
}

void ABlasterCharacter::BeginPlay()
{
	Super::BeginPlay();

	// For Server pawn on its own POV
	UpdateOverheadWidgetText();
}

void ABlasterCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	
}

void ABlasterCharacter::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();

	// For Client pawns on their own POV
	UpdateOverheadWidgetText();
}

void ABlasterCharacter::PossessedBy(AController* NewController) 
{
	Super::PossessedBy(NewController);
	
	// For Client pawns on Server POV
	UpdateOverheadWidgetText();
}

// Called to bind functionality to input
void ABlasterCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

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

	if (UEnhancedInputComponent* Input = CastChecked<UEnhancedInputComponent>(PlayerInputComponent))
	{
		if (MoveAction)
		{
			UE_LOG(LogTemp, Display, TEXT("Binding MoveInput"));
			Input->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ABlasterCharacter::MoveInput);
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("MoveAction is null!"));
		}

		if (LookAction)
		{
			UE_LOG(LogTemp, Display, TEXT("Binding LookInput"));
			Input->BindAction(LookAction, ETriggerEvent::Triggered, this, &ABlasterCharacter::LookInput);
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("LookAction is null!"));
		}

		if (JumpAction)
		{
			UE_LOG(LogTemp, Display, TEXT("Binding JumpInput"));
			Input->BindAction(JumpAction, ETriggerEvent::Started, this, &ABlasterCharacter::JumpInput);

			UE_LOG(LogTemp, Display, TEXT("Binding StopJumpInput"));
			Input->BindAction(JumpAction, ETriggerEvent::Completed, this, &ABlasterCharacter::StopJumpingInput);
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("JumpAction is null!"));
		}

		if (EquipAction)
		{
			UE_LOG(LogTemp, Display, TEXT("Binding EquipInput"));
			Input->BindAction(EquipAction, ETriggerEvent::Triggered, this, &ABlasterCharacter::EquipInput);
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("EquipAction is null!"));
		}

		if (CrouchAction)
		{
			UE_LOG(LogTemp, Display, TEXT("Binding CrouchInput"));
			Input->BindAction(CrouchAction, ETriggerEvent::Started, this, &ABlasterCharacter::CrouchInput);
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("CrouchAction is null!"));
		}

		if (AimAction)
		{
			UE_LOG(LogTemp, Display, TEXT("Binding AimInput"));
			Input->BindAction(AimAction, ETriggerEvent::Started, this, &ABlasterCharacter::AimInputStarted);
			Input->BindAction(AimAction, ETriggerEvent::Completed, this, &ABlasterCharacter::AimInputReleased);
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("AimAction is null!"));
		}
	}
}

void ABlasterCharacter::MoveInput(const FInputActionInstance& Instance)
{
	if (!Controller)
		return;

	FVector2D MovementDirection = Instance.GetValue().Get<FVector2D>();
	const FRotator Rotation(0.f, Controller->GetControlRotation().Yaw, 0.f);
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

void ABlasterCharacter::EquipInput(const FInputActionInstance& Instance)
{
	if (CombatComponent)
	{
		if (HasAuthority())
		{
			CombatComponent->EquipWeapon(OverlappingWeapon);
		}
		else
		{
			ServerEquipButtonPressed();
		}
	}
}

void ABlasterCharacter::CrouchInput(const FInputActionInstance& Instance)
{
	if (bIsCrouched)
	{
		UnCrouch();
	}
	else
	{
		Crouch();
	}
}

void ABlasterCharacter::AimInputStarted(const FInputActionInstance& Instance)
{
	if (CombatComponent)
	{
		CombatComponent->SetAiming(true);
	}
}

void ABlasterCharacter::AimInputReleased(const FInputActionInstance& Instance)
{
	if (CombatComponent)
	{
		CombatComponent->SetAiming(false);
	}
}

void ABlasterCharacter::UpdateOverheadWidgetText()
{
	if (OverheadWidget)
	{
		if (UOverheadWidget* OverheadWidgetCasted = Cast<UOverheadWidget>(OverheadWidget->GetWidget()))
		{
			OverheadWidgetCasted->ShowPlayerName(this);
		}
	}
}

void ABlasterCharacter::SetOverlappingWeapon(AWeapon* Weapon)
{
	if (OverlappingWeapon)
	{
		OverlappingWeapon->ShowPickupWidget(false);
	}
	OverlappingWeapon = Weapon;
	if (IsLocallyControlled())
	{
		if (OverlappingWeapon)
		{
			OverlappingWeapon->ShowPickupWidget(true);
		}
	}
}

bool ABlasterCharacter::IsWeaponEquipped()
{
	return CombatComponent && CombatComponent->EquippedWeapon;
}

bool ABlasterCharacter::IsAiming()
{
	return CombatComponent && CombatComponent->bAiming;
}

void ABlasterCharacter::OnRep_OverlappingWeapon(AWeapon* LastWeapon)
{
	if (OverlappingWeapon)
	{
		OverlappingWeapon->ShowPickupWidget(true);
	}
	if (LastWeapon)
	{
		LastWeapon->ShowPickupWidget(false);
	}
}

void ABlasterCharacter::ServerEquipButtonPressed_Implementation()
{
	if (CombatComponent)
	{
		CombatComponent->EquipWeapon(OverlappingWeapon);
	}
}
