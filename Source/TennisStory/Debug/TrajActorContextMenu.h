// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include <GameplayTagContainer.h>
#include "TrajActorContextMenu.generated.h"

class ATrajectoryTestActor;
class UDataTable;

UCLASS()
class TENNISSTORY_API UTrajActorContextMenu : public UUserWidget
{
	GENERATED_BODY()
	
public:
	void SetTrajActorRef(ATrajectoryTestActor* TrajActor);

protected:
	UPROPERTY(BlueprintReadOnly)
	ATrajectoryTestActor* TrajActorRef;

	UPROPERTY(EditAnywhere, Category = "Starting Trajectory")
	FGameplayTag StartingSourceTag = FGameplayTag::RequestGameplayTag(TEXT("Shot.Source.Swing"));

	UPROPERTY(EditAnywhere, Category = "Starting Trajectory")
	FGameplayTag StartingShotType = FGameplayTag::RequestGameplayTag(TEXT("Shot.Type.Topspin"));

};
