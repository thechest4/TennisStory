// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TennisRacquet.generated.h"

UCLASS()
class TENNISSTORY_API ATennisRacquet : public AActor
{
	GENERATED_BODY()
	
public:	
	ATennisRacquet();

	UPROPERTY(VisibleAnywhere)
	class USphereComponent* OverlapDetectionComp;

protected:
	UPROPERTY(VisibleAnywhere)
	FName RacquetHeadSocket = TEXT("RacquetHead");

	UPROPERTY(VisibleAnywhere)
	USkeletalMeshComponent* RacquetMesh;
};
