// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BounceLocationMarker.generated.h"

class UParticleSystemComponent;

UCLASS()
class TENNISSTORY_API ABounceLocationMarker : public AActor
{
	GENERATED_BODY()
	
public:	
	ABounceLocationMarker();

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_ShowMarkerAtLocation(FVector Location, float Duration);

	void HideMarker();

protected:
	virtual void BeginPlay() override;

	UPROPERTY(EditDefaultsOnly)
	UParticleSystemComponent* ParticleSystemComp;
};
