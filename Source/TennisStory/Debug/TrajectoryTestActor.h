// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Debug/DebugPawn.h"
#include "TrajectoryTestActor.generated.h"

class UHighlightableStaticMeshComponent;
class USplineComponent;
class USplineMeshComponent;

UCLASS()
class TENNISSTORY_API ATrajectoryTestActor : public AActor
{
	GENERATED_BODY()
	
public:	
	ATrajectoryTestActor();

	virtual void BeginPlay() override;

	virtual void Tick(float DeltaSeconds) override;

protected:
	UPROPERTY(EditAnywhere, Category = "Parameters")
	float MoveSpeed = 5.f;

	UPROPERTY(EditAnywhere, Category = "Parameters")
	UCurveFloat* TrajectoryCurve;

	UPROPERTY(EditAnywhere, Category = "Parameters")
	UStaticMesh* SplineMesh;

	UPROPERTY(VisibleAnywhere)
	UHighlightableStaticMeshComponent* TrajectorySourceComp;
	
	UPROPERTY(VisibleAnywhere)
	UHighlightableStaticMeshComponent* TrajectoryEndComp;
	
	UPROPERTY(VisibleAnywhere)
	USplineComponent* TrajectorySplineComp;

	void UpdateTrajectory();

	void UpdateSplineMesh();

	TArray<USplineMeshComponent*> SplineMeshComps;

	UPROPERTY(Transient)
	FVector SourcePrevPos;

	UPROPERTY(Transient)
	FVector EndPrevPos;
};
