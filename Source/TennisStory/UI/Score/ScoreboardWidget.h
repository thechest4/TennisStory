// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ScoreboardWidget.generated.h"

UCLASS()
class TENNISSTORY_API UScoreboardWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintImplementableEvent, Category = "Score")
	void AddSetScoreWidgets();

	void SetTeamNames(FString Team0Name, FString Team1Name);

protected:
	UPROPERTY(BlueprintReadOnly)
	FString Team0Name;
	
	UPROPERTY(BlueprintReadOnly)
	FString Team1Name;
};
