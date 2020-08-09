// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TrajectoryTestActor.generated.h"

class UHighlightableStaticMeshComponent;

UCLASS()
class TENNISSTORY_API ATrajectoryTestActor : public AActor
{
	GENERATED_BODY()
	
public:	
	ATrajectoryTestActor();

	virtual void BeginPlay() override;

protected:
	UPROPERTY(VisibleAnywhere)
	UHighlightableStaticMeshComponent* TrajectorySourceComp;
	
	UPROPERTY(VisibleAnywhere)
	UHighlightableStaticMeshComponent* TrajectoryEndComp;
};
