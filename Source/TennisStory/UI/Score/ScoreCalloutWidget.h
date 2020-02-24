// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ScoreCalloutWidget.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnCalloutWidgetFinishedEvent);

UCLASS()
class TENNISSTORY_API UScoreCalloutWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	FOnCalloutWidgetFinishedEvent& OnCalloutWidgetFinished(){ return CalloutWidgetFinishedEvent; }

	UFUNCTION(BlueprintImplementableEvent, Category = "Score")
	void ShowCalloutWidget(float Duration, const FText& HeaderText, const FText& BodyText);

	UFUNCTION(BlueprintCallable, Category = "Widget Events")
	void BroadcastWidgetFinishedEvent();

private:
	FOnCalloutWidgetFinishedEvent CalloutWidgetFinishedEvent;
};
