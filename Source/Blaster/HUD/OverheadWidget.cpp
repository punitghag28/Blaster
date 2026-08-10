// Fill out your copyright notice in the Description page of Project Settings.


#include "HUD/OverheadWidget.h"

#include "Components/TextBlock.h"

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

    ENetRole RemoteRole = InPawn->GetRemoteRole();
    FString RoleText;

    switch (RemoteRole)
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
    FString RemoteRoleString = FString::Printf(TEXT("Remote Role: %s"), *RoleText);
    SetDisplayText(RemoteRoleString);
}

void UOverheadWidget::NativeDestruct()
{
    RemoveFromParent();

    Super::NativeDestruct();
}
