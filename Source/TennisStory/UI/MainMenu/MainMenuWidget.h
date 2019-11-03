// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MainMenuWidget.generated.h"

UCLASS()
class TENNISSTORY_API UMainMenuWidget : public UUserWidget
{
	GENERATED_BODY()
	
protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString GameMapName;

	UFUNCTION(BlueprintCallable)
	void HostServer();

	UFUNCTION(BlueprintCallable)
	void JoinServer(FString argIpAddress);
};
