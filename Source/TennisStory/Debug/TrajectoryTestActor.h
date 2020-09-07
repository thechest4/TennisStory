// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Debug/DebugPawn.h"
#include "Gameplay/Ball/BallAimingFunctionLibrary.h"
#include "HasContextMenu.h"
#include "TrajectoryTestActor.generated.h"

class UHighlightableStaticMeshComponent;
class USplineComponent;
class USplineMeshComponent;
class UWidgetComponent;

UCLASS()
class TENNISSTORY_API ATrajectoryTestActor : public AActor, public IHasContextMenu
{
	GENERATED_BODY()
	
public:	
	ATrajectoryTestActor();

	virtual void BeginPlay() override;

	virtual void Tick(float DeltaSeconds) override;

	virtual void ShowContextMenu() override;

	virtual void HideContextMenu() override;

	UFUNCTION(BlueprintCallable)
	void SetCurrentTrajAlgorithm(ETrajectoryAlgorithm NewAlgo);

	UFUNCTION(BlueprintCallable)
	void SetTrajParamsOld(FTrajectoryParams_Old TrajParams);
	
	UFUNCTION(BlueprintCallable)
	void SetTrajParamsNew(FTrajectoryParams_New TrajParams);

	UFUNCTION(BlueprintCallable)
	void SetMeshLocations(FVector SourceLocation, FVector EndLocation);

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

	UPROPERTY(VisibleAnywhere)
	UWidgetComponent* ContextMenuComp;

	void UpdateTrajectory();

	void UpdateSplineMesh();

	TArray<USplineMeshComponent*> SplineMeshComps_Old;
	
	TArray<USplineMeshComponent*> SplineMeshComps_New;

	UPROPERTY(Transient)
	FVector SourcePrevPos;

	UPROPERTY(Transient)
	FVector EndPrevPos;

	ETrajectoryAlgorithm TrajAlgorithm;
	FTrajectoryParams_Old TrajParams_Old;
	FTrajectoryParams_New TrajParams_New;
};
