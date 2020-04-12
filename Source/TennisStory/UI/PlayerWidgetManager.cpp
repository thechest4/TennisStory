// Fill out your copyright notice in the Description page of Project Settings.

#include "PlayerWidgetManager.h"
#include "UI/GameMenu/GameMenuWidget.h"
#include "UI/MatchState/PlayerReadyStatusWidget.h"
#include "UI/MatchState/ReadyUpWidget.h"
#include "Framework/Application/SlateApplication.h"

UPlayerWidgetManager::UPlayerWidgetManager(const FObjectInitializer& ObjectInitializer)
	:Super(ObjectInitializer)
{
	CurrentInputMode = EInputMode::GameMode;
}

void UPlayerWidgetManager::ShowGameMenu()
{
	ensureMsgf(GameMenuWidgetObject, TEXT("UPlayerWidgetManager::ShowGameMenu - GameMenuWidgetObject was null!"));

	if (!GameMenuWidgetObject)
	{
		return;
	}

	GameMenuWidgetObject->SetVisibility(ESlateVisibility::Visible);
	
	checkf(!WidgetsWantingUIInput.Contains(GameMenuWidgetObject), TEXT("UPlayerWidgetManager::ShowGameMenu - GameMenuWidgetObject already in WidgetsWantingUIInput TArray!"))

	WidgetsWantingUIInput.Add(GameMenuWidgetObject);
	
	SetAppropriateInputMode();
}

void UPlayerWidgetManager::HideGameMenu()
{
	ensureMsgf(GameMenuWidgetObject, TEXT("UPlayerWidgetManager::HideGameMenu - GameMenuWidgetObject was null!"));
	
	if (!GameMenuWidgetObject)
	{
		return;
	}

	GameMenuWidgetObject->SetVisibility(ESlateVisibility::Collapsed);

	WidgetsWantingUIInput.Remove(GameMenuWidgetObject);
	
	SetAppropriateInputMode();
}

void UPlayerWidgetManager::ShowReadyWidgets()
{
	ensureMsgf(ReadyStateWidgetObject, TEXT("UPlayerWidgetManager::ShowReadyWidgets - ReadyStateWidgetObject was null!"));
	ensureMsgf(ReadyUpWidgetObject, TEXT("UPlayerWidgetManager::ShowReadyWidgets - ReadyUpWidgetObject was null!"));

	if (!ReadyStateWidgetObject || !ReadyUpWidgetObject)
	{
		return;
	}

	ReadyStateWidgetObject->SetVisibility(ESlateVisibility::Visible);
	ReadyStateWidgetObject->SetUpWidget();

	ReadyUpWidgetObject->SetVisibility(ESlateVisibility::Visible);
	ReadyUpWidgetObject->SetUpWidget();

	checkf(!WidgetsWantingUIInput.Contains(ReadyUpWidgetObject), TEXT("UPlayerWidgetManager::ShowReadyWidgets - ReadyUpWidget already in WidgetsWantingUIInput TArray!"))

	WidgetsWantingUIInput.Add(ReadyUpWidgetObject);
	
	SetAppropriateInputMode();
}

void UPlayerWidgetManager::HideReadyWidgets()
{
	ensureMsgf(ReadyStateWidgetObject, TEXT("UPlayerWidgetManager::HideReadyWidgets - ReadyStateWidgetObject was null!"));
	ensureMsgf(ReadyUpWidgetObject, TEXT("UPlayerWidgetManager::HideReadyWidgets - ReadyUpWidgetObject was null!"));
	
	if (!ReadyStateWidgetObject || !ReadyUpWidgetObject)
	{
		return;
	}
	
	ReadyStateWidgetObject->SetVisibility(ESlateVisibility::Collapsed);
	ReadyStateWidgetObject->CleanUpWidget();

	ReadyUpWidgetObject->SetVisibility(ESlateVisibility::Collapsed);
	ReadyUpWidgetObject->CleanUpWidget();

	WidgetsWantingUIInput.Remove(ReadyUpWidgetObject);
	
	SetAppropriateInputMode();
}

void UPlayerWidgetManager::HideAllWidgets()
{
	for (int i = 0; i < AllWidgets.Num(); i++)
	{
		AllWidgets[i]->SetVisibility(ESlateVisibility::Collapsed);
	}
}

void UPlayerWidgetManager::NativeOnInitialized()
{
	CatalogWidgetObjects();

	ensureMsgf(GameMenuWidgetObject && ReadyStateWidgetObject && ReadyUpWidgetObject, TEXT("UPlayerWidgetManager::NativeOnInitialized - Not all widgets were catalogued properly"));

	AllWidgets.Add(Cast<UUserWidget>(GameMenuWidgetObject));
	AllWidgets.Add(Cast<UUserWidget>(ReadyStateWidgetObject));
	AllWidgets.Add(Cast<UUserWidget>(ReadyUpWidgetObject));

	HideAllWidgets();

	GameMenuWidgetObject->OnGameMenuWantsClose().BindUObject(this, &UPlayerWidgetManager::HideGameMenu);
	GameMenuWidgetObject->OnReturningToMainMenu().BindUObject(this, &UPlayerWidgetManager::SetPlayerInputToGameMode);

	Super::NativeOnInitialized();
}

void UPlayerWidgetManager::SetAppropriateInputMode()
{
	if (WidgetsWantingUIInput.Num() > 0 && CurrentInputMode != EInputMode::UIMode)
	{
		SetPlayerInputToUIMode();
	}
	else if (WidgetsWantingUIInput.Num() == 0 && CurrentInputMode != EInputMode::GameMode)
	{
		SetPlayerInputToGameMode();
	}
}

void UPlayerWidgetManager::SetPlayerInputToUIMode()
{
	APlayerController* OwningPlayer = GetOwningPlayer();

	checkf(OwningPlayer, TEXT("UPlayerWidgetManager::SetPlayerInputToUIMode - OwningPlayer was null"))

	FInputModeUIOnly UIInputOnly;
	UIInputOnly.SetWidgetToFocus(TakeWidget());

	OwningPlayer->SetInputMode(UIInputOnly);
	OwningPlayer->bShowMouseCursor = true;

	CurrentInputMode = EInputMode::UIMode;
}

void UPlayerWidgetManager::SetPlayerInputToGameMode()
{
	APlayerController* OwningPlayer = GetOwningPlayer();

	checkf(GetOwningPlayer(), TEXT("UPlayerWidgetManager::SetPlayerInputToGameMode - OwningPlayer was null"))

	FInputModeGameOnly GameInputOnly;
	OwningPlayer->SetInputMode(GameInputOnly);
	OwningPlayer->bShowMouseCursor = false;
	
	CurrentInputMode = EInputMode::GameMode;
	
	FSlateApplication::Get().SetAllUserFocusToGameViewport();
}
