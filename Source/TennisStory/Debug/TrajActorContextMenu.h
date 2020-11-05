// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
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

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Trajectory Data")
	UDataTable* TrajectoryData;
};
