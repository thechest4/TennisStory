// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ServiceCalloutWidget.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnServiceCalloutWidgetFinishedEvent);

UCLASS()
class TENNISSTORY_API UServiceCalloutWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	FOnServiceCalloutWidgetFinishedEvent& OnServiceCalloutWidgetFinished(){ return ServiceCalloutWidgetFinishedEvent; }

	UFUNCTION(BlueprintImplementableEvent, Category = "Score")
	void ShowCalloutWidget(float Duration, const FText& HeaderText, const FText& BodyText);

	UFUNCTION(BlueprintCallable, Category = "Widget Events")
	void BroadcastWidgetFinishedEvent();

private:
	FOnServiceCalloutWidgetFinishedEvent ServiceCalloutWidgetFinishedEvent;
	
};
