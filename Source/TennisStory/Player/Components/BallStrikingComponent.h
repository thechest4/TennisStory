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

protected:
	UFUNCTION()
	void HandleRacquetOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector BallTrajectory = FVector(1.0f, 0.0f, 0.1f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float BallSpeed = 1000.0f;

	FRotator GetTrajectoryRotation(FVector BallLocation, FVector TargetLocation, float DesiredSpeed, float Gravity);
};
