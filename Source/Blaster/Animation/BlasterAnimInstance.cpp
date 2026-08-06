// Fill out your copyright notice in the Description page of Project Settings.


#include "Animation/BlasterAnimInstance.h"
#include "GameFramework/CharacterMovementComponent.h"

#include "Character/BlasterCharacter.h"

void UBlasterAnimInstance::NativeInitializeAnimation()
{
    Super::NativeInitializeAnimation();

    BlasterCharacter = Cast<ABlasterCharacter>(TryGetPawnOwner());
}

void UBlasterAnimInstance::NativeUpdateAnimation(float DeltaTime)
{
    Super::NativeUpdateAnimation(DeltaTime);

    if (!BlasterCharacter)
    {
        return;
    }

    FVector Velocity = BlasterCharacter->GetVelocity();
    Velocity.Z = 0.0f;
    Speed = Velocity.Size();

    bIsInAir = BlasterCharacter->GetCharacterMovement()->IsFalling();

    bIsAccelerating = BlasterCharacter->GetCharacterMovement()->GetCurrentAcceleration().Size() > 0.0f ? true : false;
}
