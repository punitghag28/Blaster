// Fill out your copyright notice in the Description page of Project Settings.


#include "HUD/OverheadWidget.h"

#include "Components/TextBlock.h"
#include "GameFramework/PlayerState.h"

void UOverheadWidget::SetDisplayText(FString TextToDisplay)
{
    if (DisplayText)
    {
        DisplayText->SetText(FText::FromString(TextToDisplay));
    }
}

void UOverheadWidget::ShowPlayerNetRole(APawn* InPawn)
{
    if (!InPawn)
    {
        return;
    }

    ENetRole LocalRole = InPawn->GetLocalRole();
    FString RoleText;

    switch (LocalRole)
    {
        case ENetRole::ROLE_Authority:
        {
            RoleText = FString("Authority");
            break;
        }
        case ENetRole::ROLE_AutonomousProxy:
        {
            RoleText = FString("Autonomous Proxy");
            break;
        }
        case ENetRole::ROLE_SimulatedProxy:
        {
            RoleText = FString("Simulated Proxy");
            break;
        }
        case ENetRole::ROLE_None:
        {
            RoleText = FString("None");
            break;
        }
    }
    FString LocalRoleString = FString::Printf(TEXT("Local Role: %s"), *RoleText);

    SetDisplayText(LocalRoleString);
}

void UOverheadWidget::ShowPlayerName(APawn* InPawn)
{
    if (!InPawn)
    {
        return;
    }

    FString PlayerName = FString("...");
    if (APlayerState* PlayerState = InPawn->GetPlayerState())
    {
        PlayerName = PlayerState->GetPlayerName();
    }

    SetDisplayText(PlayerName);
}

void UOverheadWidget::NativeDestruct()
{
    RemoveFromParent();

    Super::NativeDestruct();
}
