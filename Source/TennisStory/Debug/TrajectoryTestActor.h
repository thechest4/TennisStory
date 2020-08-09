// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Debug/DebugPawn.h"
#include "TrajectoryTestActor.generated.h"

class UHighlightableStaticMeshComponent;

UCLASS()
class TENNISSTORY_API ATrajectoryTestActor : public AActor
{
	GENERATED_BODY()
	
public:	
	ATrajectoryTestActor();

	virtual void BeginPlay() override;

	void RequestMoveComponent(UHighlightableStaticMeshComponent* CompToMove, EMouseMoveType MoveType, FVector RightVector, float XDelta, float YDelta);

protected:
	UPROPERTY(EditAnywhere, Category = "Parameters")
	float MoveSpeed = 5.f;

	UPROPERTY(VisibleAnywhere)
	UHighlightableStaticMeshComponent* TrajectorySourceComp;

	TArray<EMouseMoveType> AllowedMoveTypes_TrajectorySource;
	
	UPROPERTY(VisibleAnywhere)
	UHighlightableStaticMeshComponent* TrajectoryEndComp;
	
	TArray<EMouseMoveType> AllowedMoveTypes_TrajectoryEnd;
};
