// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "DistanceIndicatorComponent.generated.h"

DECLARE_MULTICAST_DELEGATE(FTargetReachedEvent);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class TENNISSTORY_API UDistanceIndicatorComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UDistanceIndicatorComponent();

	UPROPERTY(Transient)
	USceneComponent* VisualComp;

	bool IsVisualizingDistance() { return bIsVisualizingDistance; }

	void StartVisualizingDistance(TWeakObjectPtr<AActor> ActorToTrack);

	void StopVisualizingDistance();

	FTargetReachedEvent& OnTargetReached() { return TargetReachedEvent; }

protected:
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	
	UPROPERTY(EditDefaultsOnly, Category = "Distance Indicator")
	float MinDistanceForVisualization;

	UPROPERTY(EditDefaultsOnly, Category = "Distance Indicator")
	float DesiredDistance;
	
	UPROPERTY(EditDefaultsOnly, Category = "Distance Indicator")
	float MaxScale;
	
	UPROPERTY(EditDefaultsOnly, Category = "Distance Indicator")
	float MinScale;

	TWeakObjectPtr<AActor> ActorToTrack;

	bool bIsVisualizingDistance;
	float CurrentAlpha;
	float StartingDistance;
	float DistanceDifference;

	FTargetReachedEvent TargetReachedEvent;
};
