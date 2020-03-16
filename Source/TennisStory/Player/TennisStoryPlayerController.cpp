// Fill out your copyright notice in the Description page of Project Settings.

#include "TennisStoryPlayerController.h"
#include "Net/UnrealNetwork.h"
#include "UI/GameMenu/GameMenuWidget.h"

void ATennisStoryPlayerController::OnPossess(APawn* aPawn)
{
	Super::OnPossess(aPawn);

	//NOTE(achester): This is important so that the far-court player still controls like the near court player despite facing toward the camera.
	//This will probably need to change to support different cameras
	SetControlRotation(FRotator::ZeroRotator);
}

void ATennisStoryPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();
	
	InputComponent->BindAction("ToggleGameMenu", IE_Pressed, this, &ATennisStoryPlayerController::ShowGameMenu);
}

void ATennisStoryPlayerController::ShowGameMenu()
{
	if (!GameMenuClass || !IsPrimaryPlayer())
	{
		return;
	}

	if (!GameMenuObject)
	{
		GameMenuObject = CreateWidget<UGameMenuWidget>(GetWorld(), GameMenuClass);
		GameMenuObject->AddToViewport(10);
		GameMenuObject->SetOwningPlayer(this);
	}

	checkf(GameMenuObject, TEXT("ATennisStoryPlayerController::ShowGameMenu - GameMenuObject was null!"))

	if (GameMenuObject->GetVisibility() != ESlateVisibility::Visible)
	{
		GameMenuObject->SetVisibility(ESlateVisibility::Visible);
		
		FInputModeUIOnly UIInputOnly;
		UIInputOnly.SetWidgetToFocus(GameMenuObject->TakeWidget());

		SetInputMode(UIInputOnly);
		bShowMouseCursor = true;
	}
}

void ATennisStoryPlayerController::HideGameMenu()
{
	if (GameMenuObject)
	{
		GameMenuObject->SetVisibility(ESlateVisibility::Collapsed);

		FInputModeGameOnly GameOnlyInput;
		SetInputMode(GameOnlyInput);
		bShowMouseCursor = false;
	}
}
