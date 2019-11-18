// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TennisBall.generated.h"

class ATennisStoryCharacter;

UENUM(BlueprintType)
enum class ETennisBallState : uint8
{
	ServiceState,
	PlayState
};

UCLASS()
class TENNISSTORY_API ATennisBall : public AActor
{
	GENERATED_BODY()
	
public:	
	ATennisBall();
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	class UBallMovementComponent* BallMovementComp;

	UFUNCTION(BlueprintCallable, Category = "Tennis Ball")
	bool IsInServiceState();

	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Tennis Ball")
	void SetBallState(ETennisBallState NewState);

	TWeakObjectPtr<ATennisStoryCharacter> LastPlayerToHit;

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_FollowPath(USplineComponent* PathProviderComp, float Velocity, UCurveFloat* TrajectoryCurve);

protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* BallMesh;

	ETennisBallState CurrentBallState;

	void ApplyBallState();
};
