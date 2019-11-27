// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Gameplay/HalfCourt.h"
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

	void ShowTargetOnCourt(TWeakObjectPtr<class AHalfCourt> CourtToAimAt, bool bShowTarget);

	void DisableTargetMovement();

	void HideTarget();

	FVector GetDesiredLocationOffset() const
	{
		return FVector(0.f, 0.f, 0.1f);
	}

protected:
	virtual void BeginPlay() override;

	bool bCurrentlyVisible;
	bool bCurrentlyMovable;
	float TargetingStartedTime;
	TWeakObjectPtr<AHalfCourt> CurrentTargetCourt;
	ESnapPoint LastSnapPoint;

	UPROPERTY(VisibleDefaultsOnly, Category = "Player Target")
	UStaticMeshComponent* TargetMesh;

	UPROPERTY(EditAnywhere, Category = "Player Target")
	float MoveSpeed = 3.0f;

	UPROPERTY(EditAnywhere, Category = "Player Target")
	float LockedTargetingDuration = 1.0f;

	FVector CurrentInputVector = FVector::ZeroVector;

	FVector ConsumeCurrentInputVector();

	FVector GetOwnerControlRotationVector();
};
