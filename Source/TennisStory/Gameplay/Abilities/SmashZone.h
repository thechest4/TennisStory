// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SmashZone.generated.h"

class USphereComponent;
class ATennisStoryCharacter;

UCLASS()
class TENNISSTORY_API ASmashZone : public AActor
{
	GENERATED_BODY()
	
public:	
	ASmashZone();

protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	void HandleSphereBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void HandleSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	UPROPERTY(EditAnywhere)
	USphereComponent* PlayerDetectionSphere;

	UPROPERTY(EditAnywhere)
	UStaticMeshComponent* RingMesh;

	UPROPERTY(EditAnywhere)
	UMaterialInterface* NormalMaterial; //Material when a player is not detected

	UPROPERTY(EditAnywhere)
	UMaterialInterface* OccupiedMaterial; //Material when a player is detected

	TArray<TWeakObjectPtr<ATennisStoryCharacter>> AffectedPlayers;
};
