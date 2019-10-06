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

	void ShowTargetOnCourt(TWeakObjectPtr<class AHalfCourt> CourtToAimAt);

	void HideTarget();

protected:
	bool bCurrentlyVisible;
	float TargetingStartedTime;
	TWeakObjectPtr<AHalfCourt> CurrentTargetCourt;

	UPROPERTY(VisibleDefaultsOnly, Category = "Player Target")
	UStaticMeshComponent* TargetMesh;

	UPROPERTY(EditAnywhere, Category = "Player Target")
	float MoveSpeed = 3.0f;

	UPROPERTY(EditAnywhere, Category = "Player Target")
	float LockedTargetingDuration = 0.5f;

	FVector LastInputVector = FVector::ZeroVector;
	FVector CurrentInputVector = FVector::ZeroVector;

	FVector ConsumeCurrentInputVector();
};
