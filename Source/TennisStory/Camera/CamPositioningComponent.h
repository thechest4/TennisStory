// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CamPositioningComponent.generated.h"


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
	float PositioningSpeed = 500.f;

	TWeakObjectPtr<AActor> OwnerPtr;
	TWeakObjectPtr<class UCameraComponent> OwnerCamComp;

	TArray<TWeakObjectPtr<AActor>> TrackedActors;
};
