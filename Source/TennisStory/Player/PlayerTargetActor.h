// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PlayerTargetActor.generated.h"

UCLASS()
class TENNISSTORY_API APlayerTargetActor : public AActor
{
	GENERATED_BODY()
	
public:	
	APlayerTargetActor();

	float const GetMoveSpeed() { return MoveSpeed; };

	void AddInputVector(FVector Direction, float Value);

	void Tick(float DeltaSeconds) override;

protected:
	UPROPERTY(EditAnywhere)
	float MoveSpeed = 3.0f;

	FVector LastInputVector = FVector::ZeroVector;
	FVector CurrentInputVector = FVector::ZeroVector;

	FVector ConsumeCurrentInputVector();
};
