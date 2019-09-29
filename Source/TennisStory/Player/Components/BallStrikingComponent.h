// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "BallStrikingComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class TENNISSTORY_API UBallStrikingComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UBallStrikingComponent();

	void AllowBallStriking();

	void StopBallStriking();

	void SetChargeStartTime();

	void SetChargeEndTime();

protected:
	UFUNCTION()
	void HandleRacquetOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MinBallSpeed = 1000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MaxBallSpeed = 3000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MaxChargeDuration = 3.0f;

	FRotator GetTrajectoryRotation(FVector BallLocation, FVector TargetLocation, float DesiredSpeed, float Gravity);

	float CalculateChargedBallSpeed();

	//Charge State
	float LastChargeStartTime = 0.0f;
	float LastChargeEndTime = 0.0f;
};
