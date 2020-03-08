// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "TennisStoryPlayerState.generated.h"

DECLARE_MULTICAST_DELEGATE_OneParam(FOnReadyStateUpdatedEvent, ATennisStoryPlayerState*)

UCLASS()
class TENNISSTORY_API ATennisStoryPlayerState : public APlayerState
{
	GENERATED_BODY()

public:
	ATennisStoryPlayerState(const FObjectInitializer& ObjectInitializer);

	FOnReadyStateUpdatedEvent& OnReadyStateUpdated(){ return ReadyStateUpdatedEvent; }

	UFUNCTION(BlueprintCallable, Category = "Player State")
	bool IsReady()
	{
		return bIsReady;
	}
	
protected:
	UFUNCTION(Server, Reliable, WithValidation)
	void Server_UpdateIsReady(bool bNewReady);

	UPROPERTY(ReplicatedUsing = OnRep_IsReady)
	bool bIsReady;

	UFUNCTION()
	void OnRep_IsReady();
	
	FOnReadyStateUpdatedEvent ReadyStateUpdatedEvent;

	friend class UReadyUpWidget;
};
