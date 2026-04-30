// Fill out your copyright notice in the Description page of Project Settings.


#include "Menu.h"

#include "Components/Button.h"
#include "MultiplayerSessionsSubsystem.h"
#include "OnlineSessionSettings.h"
#include "Online/OnlineSessionNames.h"
#include "Interfaces/OnlineSessionInterface.h"

void UMenu::MenuSetup(int32 noOfPublicConnections /*default = 4*/, FString TypeMatch /*default = FString(TEXT("FreeForAll"))*/, FString LobbyPath /*default = /Game/ThirdPerson/Maps/Lobby*/)
{
    NumPublicConnections = noOfPublicConnections;
    TypeOfMatch = TypeMatch;
    PathToLobby = FString::Printf(TEXT("%s?listen"), *LobbyPath);
    AddToViewport();
    SetVisibility(ESlateVisibility::Visible);
    SetIsFocusable(true);

    if (UWorld* World = GetWorld())
    {
        if (APlayerController* PlayerController = World->GetFirstPlayerController())
        {
            FInputModeUIOnly InputModeData;
            InputModeData.SetWidgetToFocus(TakeWidget());
            InputModeData.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);

            PlayerController->SetInputMode(InputModeData);
            PlayerController->SetShowMouseCursor(true);
        }
        
    }

    if (UGameInstance* GameInstance = GetGameInstance())
    {
        MultiplayerSessionsSubsystem = GameInstance->GetSubsystem<UMultiplayerSessionsSubsystem>();
    }

    if (MultiplayerSessionsSubsystem)
    {
        MultiplayerSessionsSubsystem->MultiplayerOnCreateSessionCompleteDelegate.AddDynamic(this, &ThisClass::OnCreateSession);
        MultiplayerSessionsSubsystem->MultiplayerOnFindSessionsCompleteDelegate.AddUObject(this, &ThisClass::OnFindSessions);
        MultiplayerSessionsSubsystem->MultiplayerOnJoinSessionCompleteDelegate.AddUObject(this, &ThisClass::OnJoinSession);
        MultiplayerSessionsSubsystem->MultiplayerOnStartSessionCompleteDelegate.AddDynamic(this, &ThisClass::OnStartSession);
        MultiplayerSessionsSubsystem->MultiplayerOnDestroySessionCompleteDelegate.AddDynamic(this, &ThisClass::OnDestroySession);
    }
}

bool UMenu::Initialize()
{
    if (!Super::Initialize())
    {
        return false;
    }

    if (HostButton)
    {
        HostButton->OnClicked.AddDynamic(this, &ThisClass::HostButtonClicked);
    }

    if (JoinButton)
    {
        JoinButton->OnClicked.AddDynamic(this, &ThisClass::JoinButtonClicked);
    }

    return true;
}

void UMenu::NativeDestruct()
{
    MenuTeardown();

    Super::NativeDestruct();
}

void UMenu::OnCreateSession(bool bWasSuccessful)
{
    if (GEngine)
    {
        if (bWasSuccessful)
        {
            GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Green, FString(TEXT("Session Created Successfully!")));

            if (UWorld* World = GetWorld())
            {
                World->ServerTravel(PathToLobby);
                MultiplayerSessionsSubsystem->StartSession();
            }

        }
        else
        {
            GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, FString(TEXT("Session Creation Failed!")));
            HostButton->SetIsEnabled(true);
        }
    }
}

void UMenu::OnFindSessions(const TArray<FOnlineSessionSearchResult>& SessionResults, bool bWasSuccessful)
{
    if (!MultiplayerSessionsSubsystem)
    {
        JoinButton->SetIsEnabled(true);
        return;
    }

    if (bWasSuccessful)
    {
        for (auto Result : SessionResults)
        {
            FString MatchType;
            Result.Session.SessionSettings.Get(SETTING_GAMEMODE, MatchType);

            if (MatchType == "FreeForAll")
            {
                MultiplayerSessionsSubsystem->JoinSession(Result);
                return;
            }
        }
    }
    JoinButton->SetIsEnabled(true);

}

void UMenu::OnJoinSession(EOnJoinSessionCompleteResult::Type Result)
{
    if (Result != EOnJoinSessionCompleteResult::Success)
    {
        JoinButton->SetIsEnabled(true);
    }

    if (IOnlineSubsystem* Subsystem = IOnlineSubsystem::Get())
    {
        IOnlineSessionPtr OnlineSessionInterfacePtr = Subsystem->GetSessionInterface();
        if (!OnlineSessionInterfacePtr.IsValid())
        {
            return;
        }

        FString Address;
        if (OnlineSessionInterfacePtr->GetResolvedConnectString(NAME_GameSession, Address))
        {
            if (APlayerController* PlayerController = GetGameInstance()->GetFirstLocalPlayerController())
            {
                PlayerController->ClientTravel(Address, ETravelType::TRAVEL_Absolute);
            }
        }
    }

}

void UMenu::OnStartSession(bool bWasSuccessful)
{
    if (GEngine)
    {
        if (bWasSuccessful)
        {
            GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Green, FString(TEXT("Session Started Successfully!")));

        }
        else
        {
            GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, FString(TEXT("Session failed to start")));
        }
    }
}

void UMenu::OnDestroySession(bool bWasSuccessful)
{
    if (GEngine)
    {
        if (bWasSuccessful)
        {
            GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Green, FString(TEXT("Session Destroyed Successfully!")));

        }
        else
        {
            GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, FString(TEXT("Session failed to destroy")));
        }
    }
}

void UMenu::HostButtonClicked()
{
    HostButton->SetIsEnabled(false);
    if (MultiplayerSessionsSubsystem)
    {
        MultiplayerSessionsSubsystem->CreateSession(NumPublicConnections, TypeOfMatch);
    }
}

void UMenu::JoinButtonClicked()
{
    JoinButton->SetIsEnabled(false);
    if (MultiplayerSessionsSubsystem)
    {
        MultiplayerSessionsSubsystem->FindSessions(1000);
    }
}

void UMenu::MenuTeardown()
{
    RemoveFromParent();
    if (UWorld* World = GetWorld())
    {
        if (APlayerController* PlayerController = World->GetFirstPlayerController())
        {
            FInputModeGameOnly InputModeData;

            PlayerController->SetInputMode(InputModeData);
            PlayerController->SetShowMouseCursor(false);
        }
    }
}
