// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TennisBall.generated.h"

UCLASS()
class TENNISSTORY_API ATennisBall : public AActor
{
	GENERATED_BODY()
	
public:	
	ATennisBall();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	class UProjectileMovementComponent* ProjMovementComp;

	UFUNCTION(BlueprintCallable, Category = "Tennis Ball")
	bool IsInServiceState();

	UFUNCTION(BlueprintCallable, Category = "Tennis Ball")
	void SetBallStateForService();

	UFUNCTION(BlueprintCallable, Category = "Tennis Ball")
	void SetBallStateForPlay();

protected:
	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* BallMesh;
};
