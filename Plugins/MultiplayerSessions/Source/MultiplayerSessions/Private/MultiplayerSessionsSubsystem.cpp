// Fill out your copyright notice in the Description page of Project Settings.


#include "MultiplayerSessionsSubsystem.h"

#include "OnlineSubsystem.h"
#include "OnlineSessionSettings.h"
#include "Online/OnlineSessionNames.h"

UMultiplayerSessionsSubsystem::UMultiplayerSessionsSubsystem():
    CreateSessionCompleteDelegate(FOnCreateSessionCompleteDelegate::CreateUObject(this, &ThisClass::OnCreateSessionComplete)),
    FindSessionsCompleteDelegate(FOnFindSessionsCompleteDelegate::CreateUObject(this, &ThisClass::OnFindSessionsComplete)),
    JoinSessionCompleteDelegate(FOnJoinSessionCompleteDelegate::CreateUObject(this, &ThisClass::OnJoinSessionComplete)),
    StartSessionCompleteDelegate(FOnStartSessionCompleteDelegate::CreateUObject(this, &ThisClass::OnStartSessionComplete)),
    DestroySessionCompleteDelegate(FOnDestroySessionCompleteDelegate::CreateUObject(this, &ThisClass::OnDestroySessionComplete))
{
    if (IOnlineSubsystem* Subsystem = IOnlineSubsystem::Get())
    {
        OnlineSessionPtr = Subsystem->GetSessionInterface();
    }

}

void UMultiplayerSessionsSubsystem::CreateSession(int32 NumPublicConnections, FString MatchType)
{
	if (!OnlineSessionPtr.IsValid())
	{
		return;
	}

	if (FNamedOnlineSession* ExistingSession = OnlineSessionPtr->GetNamedSession(NAME_GameSession))
	{
		bCreateSessionOnDestroy = true;
		LastNumPublicConnections = NumPublicConnections;
		LastMatchType = MatchType;

		DestroySession();
		return;
	}

	// Storing the delegate in a delegate handler so we can remove it later from the delegate list
	CreateSessionCompleteDelegateHandle = OnlineSessionPtr->AddOnCreateSessionCompleteDelegate_Handle(CreateSessionCompleteDelegate);

	LastSessionSettingsPtr = MakeShareable(new FOnlineSessionSettings());
	LastSessionSettingsPtr->bIsLANMatch = IOnlineSubsystem::Get()->GetSubsystemName() == "NULL" ? true : false;
	LastSessionSettingsPtr->NumPublicConnections = NumPublicConnections;
	LastSessionSettingsPtr->bAllowJoinInProgress = true;
	LastSessionSettingsPtr->bAllowJoinViaPresence = true;
	LastSessionSettingsPtr->bShouldAdvertise = true;
	LastSessionSettingsPtr->bUsesPresence = true;
	LastSessionSettingsPtr->bUseLobbiesIfAvailable = true;
	LastSessionSettingsPtr->Set(SETTING_GAMEMODE, MatchType, EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);
	LastSessionSettingsPtr->BuildUniqueId = 1;

	const ULocalPlayer* LocalPlayer = GetWorld()->GetFirstLocalPlayerFromController();
	if (!OnlineSessionPtr->CreateSession(*LocalPlayer->GetPreferredUniqueNetId(), NAME_GameSession, *LastSessionSettingsPtr))
	{
		OnlineSessionPtr->ClearOnCreateSessionCompleteDelegate_Handle(CreateSessionCompleteDelegateHandle);

		MultiplayerOnCreateSessionCompleteDelegate.Broadcast(false);
	}
}

void UMultiplayerSessionsSubsystem::FindSessions(int32 MaxSearchResults)
{
	if (!OnlineSessionPtr)
	{
		return;
	}

	FindSessionsCompleteDelegateHandle = OnlineSessionPtr->AddOnFindSessionsCompleteDelegate_Handle(FindSessionsCompleteDelegate);

    LastSessionSearchedPtr = MakeShareable(new FOnlineSessionSearch());
    LastSessionSearchedPtr->MaxSearchResults = MaxSearchResults;
    LastSessionSearchedPtr->bIsLanQuery = IOnlineSubsystem::Get()->GetSubsystemName() == "NULL" ? true : false;
    LastSessionSearchedPtr->QuerySettings.Set(SEARCH_LOBBIES, true, EOnlineComparisonOp::Equals);

	const ULocalPlayer* LocalPlayer = GetWorld()->GetFirstLocalPlayerFromController();

	if (!OnlineSessionPtr->FindSessions(*LocalPlayer->GetPreferredUniqueNetId(), LastSessionSearchedPtr.ToSharedRef()))
	{
		OnlineSessionPtr->ClearOnFindSessionsCompleteDelegate_Handle(FindSessionsCompleteDelegateHandle);
		MultiplayerOnFindSessionsCompleteDelegate.Broadcast(TArray<FOnlineSessionSearchResult>(), false);
	}
}

void UMultiplayerSessionsSubsystem::JoinSession(const FOnlineSessionSearchResult& SessionResult)
{
	if (!OnlineSessionPtr.IsValid())
	{
		MultiplayerOnJoinSessionCompleteDelegate.Broadcast(EOnJoinSessionCompleteResult::UnknownError);
		return;
	}

	JoinSessionCompleteDelegateHandle = OnlineSessionPtr->AddOnJoinSessionCompleteDelegate_Handle(JoinSessionCompleteDelegate);

	const ULocalPlayer* LocalPlayer = GetWorld()->GetFirstLocalPlayerFromController();
	if (!OnlineSessionPtr->JoinSession(*LocalPlayer->GetPreferredUniqueNetId(), NAME_GameSession, SessionResult))
	{
		OnlineSessionPtr->ClearOnJoinSessionCompleteDelegate_Handle(JoinSessionCompleteDelegateHandle);

		MultiplayerOnJoinSessionCompleteDelegate.Broadcast(EOnJoinSessionCompleteResult::UnknownError);
	}
}

void UMultiplayerSessionsSubsystem::DestroySession()
{
	if (!OnlineSessionPtr)
	{
		MultiplayerOnDestroySessionCompleteDelegate.Broadcast(false);
		return;
	}

	DestroySessionCompleteDelegateHandle = OnlineSessionPtr->AddOnDestroySessionCompleteDelegate_Handle(DestroySessionCompleteDelegate);

	if (!OnlineSessionPtr->DestroySession(NAME_GameSession))
	{
		OnlineSessionPtr->ClearOnDestroySessionCompleteDelegate_Handle(DestroySessionCompleteDelegateHandle);
		MultiplayerOnDestroySessionCompleteDelegate.Broadcast(false);
	}
}

void UMultiplayerSessionsSubsystem::StartSession()
{
	if (!OnlineSessionPtr.IsValid())
	{
		MultiplayerOnStartSessionCompleteDelegate.Broadcast(false);
		return;
	}

	StartSessionCompleteDelegateHandle = OnlineSessionPtr->AddOnStartSessionCompleteDelegate_Handle(StartSessionCompleteDelegate);

	if (!OnlineSessionPtr->StartSession(NAME_GameSession))
	{
		OnlineSessionPtr->ClearOnStartSessionCompleteDelegate_Handle(StartSessionCompleteDelegateHandle);
		MultiplayerOnStartSessionCompleteDelegate.Broadcast(false);
	}
}

void UMultiplayerSessionsSubsystem::OnCreateSessionComplete(FName SessionName, bool bWasSuccessful)
{
	if (OnlineSessionPtr)
	{
		OnlineSessionPtr->ClearOnCreateSessionCompleteDelegate_Handle(CreateSessionCompleteDelegateHandle);
	}

	MultiplayerOnCreateSessionCompleteDelegate.Broadcast(bWasSuccessful);
}

void UMultiplayerSessionsSubsystem::OnFindSessionsComplete(bool bWasSuccessful)
{
	if (OnlineSessionPtr)
	{
		OnlineSessionPtr->ClearOnFindSessionsCompleteDelegate_Handle(FindSessionsCompleteDelegateHandle);
	}

	if (LastSessionSearchedPtr->SearchResults.Num() <= 0)
	{
		MultiplayerOnFindSessionsCompleteDelegate.Broadcast(TArray<FOnlineSessionSearchResult>(), false);
	}
	else
	{
		MultiplayerOnFindSessionsCompleteDelegate.Broadcast(LastSessionSearchedPtr->SearchResults, bWasSuccessful);
	}

}

void UMultiplayerSessionsSubsystem::OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result)
{
	if (OnlineSessionPtr)
	{
		OnlineSessionPtr->ClearOnJoinSessionCompleteDelegate_Handle(JoinSessionCompleteDelegateHandle);

		MultiplayerOnJoinSessionCompleteDelegate.Broadcast(Result);
	}
}

void UMultiplayerSessionsSubsystem::OnStartSessionComplete(FName SessionName, bool bWasSuccessful)
{
	if (OnlineSessionPtr)
	{
		OnlineSessionPtr->ClearOnStartSessionCompleteDelegate_Handle(StartSessionCompleteDelegateHandle);
	}

	MultiplayerOnStartSessionCompleteDelegate.Broadcast(bWasSuccessful);
}

void UMultiplayerSessionsSubsystem::OnDestroySessionComplete(FName SessionName, bool bWasSuccessful)
{
	if (OnlineSessionPtr)
	{
		OnlineSessionPtr->ClearOnDestroySessionCompleteDelegate_Handle(DestroySessionCompleteDelegateHandle);
	}

	if (bWasSuccessful && bCreateSessionOnDestroy)
	{
		bCreateSessionOnDestroy = false;
		CreateSession(LastNumPublicConnections, LastMatchType);
	}

	MultiplayerOnDestroySessionCompleteDelegate.Broadcast(bWasSuccessful);
}
