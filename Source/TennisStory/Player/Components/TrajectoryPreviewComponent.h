// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Gameplay/Ball/BallAimingFunctionLibrary.h"
#include "TrajectoryPreviewComponent.generated.h"

class USplineMeshComponent;
class USplineComponent;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class TENNISSTORY_API UTrajectoryPreviewComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UTrajectoryPreviewComponent();

	void StartShowingTrajectory(USplineComponent* SplinePreviewComp, UObject* StartLocObj, UObject* EndLocObj, FName TrajParamsRowName);

	void StopShowingTrajectory();

protected:
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	void GeneratePreviewTrajectory();

	FVector GetObjectLocation(UObject* Object);

	TWeakObjectPtr<USplineComponent> OwnerSplinePreviewComp;
	TWeakObjectPtr<UObject> StartLocObjPtr;
	TWeakObjectPtr<UObject> EndLocObjPtr;

	FName CurrentTrajParamsRowName;

	TArray<USplineMeshComponent*> SplineMeshComps;

	UPROPERTY(EditDefaultsOnly)
	UStaticMesh* SplineMesh;

	UPROPERTY(EditDefaultsOnly)
	UMaterialInterface* ValidSplineMeshMat;

	UPROPERTY(EditDefaultsOnly)
	UMaterialInterface* InvalidSplineMeshMat;
		
	bool bIsCurrentlyShowingTrajectory;

	FVector PrevStartLocation;
	FVector PrevEndLocation;
};
