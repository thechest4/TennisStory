// Fill out your copyright notice in the Description page of Project Settings.

#include "TennisStoryPlayerController.h"
#include "Net/UnrealNetwork.h"
#include "UI/PlayerWidgetManager.h"

void ATennisStoryPlayerController::SetPlayer(UPlayer* InPlayer)
{
	Super::SetPlayer(InPlayer);

	//This is the first place our ULocalPlayer is available, so it's the first place we can call UUserWidget::SetOwningPlayer
	CreatePlayerWidgetManager();
}

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

void ATennisStoryPlayerController::CreatePlayerWidgetManager()
{
	if (!WidgetManagerClass || !IsPrimaryPlayer())
	{
		return;
	}

	if (!WidgetManagerObject)
	{
		WidgetManagerObject = CreateWidget<UPlayerWidgetManager>(GetWorld(), WidgetManagerClass);
		WidgetManagerObject->AddToViewport(10);
		WidgetManagerObject->SetOwningPlayer(this);
	}
}

void ATennisStoryPlayerController::ShowGameMenu()
{
	ensureMsgf(WidgetManagerObject, TEXT("ATennisStoryPlayerController::ShowGameMenu - WidgetManagerObject was null!"));

	if (WidgetManagerObject)
	{
		WidgetManagerObject->ShowGameMenu();
	}
}

void ATennisStoryPlayerController::HideGameMenu()
{
	ensureMsgf(WidgetManagerObject, TEXT("ATennisStoryPlayerController::HideGameMenu - WidgetManagerObject was null!"));

	if (WidgetManagerObject)
	{
		WidgetManagerObject->HideGameMenu();
	}
}
