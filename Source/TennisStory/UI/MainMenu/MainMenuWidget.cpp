// Fill out your copyright notice in the Description page of Project Settings.

#include "MainMenuWidget.h"

void UMainMenuWidget::HostServer()
{
	FString TravelStr = GameMapName + "?listen";
	GetWorld()->ServerTravel(TravelStr);
}

void UMainMenuWidget::JoinServer(FString argIpAddress)
{
	FString TravelStr = argIpAddress + ":7777";
	GetWorld()->GetFirstPlayerController()->ClientTravel(TravelStr, ETravelType::TRAVEL_Absolute);
}
