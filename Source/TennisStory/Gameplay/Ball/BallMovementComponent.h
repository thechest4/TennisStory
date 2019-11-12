// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "BallMovementComponent.generated.h"

class USplineComponent;
class USplineMeshComponent;
class UCurveFloat;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class TENNISSTORY_API UBallMovementComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UBallMovementComponent();

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UFUNCTION(BlueprintCallable)
	void SetSplineComp(USplineComponent* SplineComp);

	UFUNCTION(BlueprintCallable)
	void GenerateTrajectory(FVector TargetLocation);

	UFUNCTION(BlueprintCallable)
	void VisualizePath();

	UFUNCTION(BlueprintCallable)
	void StartFollowingPath();

protected:
	virtual void BeginPlay() override;

	UPROPERTY(EditDefaultsOnly, Category = "Ball Trajectory")
	UCurveFloat* FlatTrajectoryCurve;

	UPROPERTY()
	USplineComponent* SplineComp;

	UPROPERTY(EditDefaultsOnly, Category = "Trajectory Visualization")
	UStaticMesh* SplineMesh;

	UPROPERTY()
	TArray<USplineMeshComponent*> SplineMeshComps;

	bool bIsFollowingPath;

	UPROPERTY(EditAnywhere, Category = "Ball Trajectory")
	float Velocity = 1500.f;

	UFUNCTION()
	void HandleActorHit(AActor* SelfActor, AActor* OtherActor, FVector NormalImpulse, const FHitResult& Hit);
};
