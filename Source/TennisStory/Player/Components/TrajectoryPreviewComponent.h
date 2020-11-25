// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Gameplay/Ball/BallAimingFunctionLibrary.h"
#include "TrajectoryPreviewComponent.generated.h"

class USplineMeshComponent;
class USplineComponent;
class UBallStrikingComponent;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class TENNISSTORY_API UTrajectoryPreviewComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UTrajectoryPreviewComponent();

	void SetBallStrikingCompReference(UBallStrikingComponent* BallStrikingComp);

	void StartShowingTrajectory(USplineComponent* SplinePreviewComp, UObject* StartLocObj, UObject* EndLocObj);

	void StopShowingTrajectory();

protected:
	void HandleShotTagsChanged();

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	void GeneratePreviewTrajectory();

	FVector GetObjectLocation(UObject* Object);

	TWeakObjectPtr<UBallStrikingComponent> OwnerBallStrikingComp;

	TWeakObjectPtr<USplineComponent> OwnerSplinePreviewComp;
	TWeakObjectPtr<UObject> StartLocObjPtr;
	TWeakObjectPtr<UObject> EndLocObjPtr;

	FTrajectoryParams CurrentTrajParams;

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
