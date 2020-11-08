// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TennisNetActor.generated.h"

class UBoxComponent;

UCLASS()
class TENNISSTORY_API ATennisNetActor : public AActor
{
	GENERATED_BODY()
	
public:	
	ATennisNetActor();

protected:
	UPROPERTY(EditAnywhere)
	UStaticMeshComponent* NetMesh;

	UPROPERTY(EditAnywhere)
	UBoxComponent* BoxCollision;
};
