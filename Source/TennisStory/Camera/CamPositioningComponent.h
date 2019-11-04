// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CamPositioningComponent.generated.h"

class UCurveFloat;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class TENNISSTORY_API UCamPositioningComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UCamPositioningComponent();

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	void AddTrackedActor(AActor* ActorToTrack);

protected:
	virtual void BeginPlay() override;

	UPROPERTY(EditDefaultsOnly, Category = "Camera Positioning")
	//The desired distance from the edge of the screen for each tracked point, as a percentage
	float MarginFromScreenEdges = 0.05f;

	UPROPERTY(EditDefaultsOnly, Category = "Camera Positioning")
	float ApproachMargin = 0.08f;

	UPROPERTY(EditDefaultsOnly, Category = "Camera Positioning")
	UCurveFloat* HorizontalPositioningSpeedCurve;

	UPROPERTY(EditDefaultsOnly, Category = "Camera Positioning")
	UCurveFloat* PullBackSpeedCurve;

	UPROPERTY(EditDefaultsOnly, Category = "Camera Positioning")
	UCurveFloat* ApproachSpeedCurve;

	const float FallbackPositioningSpeed = 500.f;

	TWeakObjectPtr<AActor> OwnerPtr;
	TWeakObjectPtr<class UCameraComponent> OwnerCamComp;

	UPROPERTY(Replicated)
	TArray<TWeakObjectPtr<AActor>> TrackedActors;

	UPROPERTY(EditDefaultsOnly, Category = "Camera Positioning")
	float MoveDuration = 0.2f;

	int CurrentLateralDirectionSign = 0;
	float LastLateralMoveTimestamp = -1.f;

	int CurrentForwardDirectionSign = 0;
	float LastForwardMoveTimestamp = -1.f;

	int CurrentVerticalDirectionSign = 0;
	float LastVerticalMoveTimestamp = -1.f;
};
