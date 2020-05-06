// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PlayerMouseTarget.generated.h"

UCLASS()
class TENNISSTORY_API APlayerMouseTarget : public AActor
{
	GENERATED_BODY()
	
public:	
	APlayerMouseTarget();

	virtual void BeginPlay() override;

	virtual void Tick(float DeltaTime) override;

	float GetRightMovementSpeed() { return RightMovementSpeed; }
	
	float GetForwardMovementSpeed() { return ForwardMovementSpeed; }

	bool IsCurrentlyShown() { return bIsCurrentlyShown; }

	void ShowTarget();

	void HideTarget();

	void AddForwardInput(float Value);

	void AddRightInput(float Value);

	FVector2D ConsumeInputVector();

protected:
	UPROPERTY(EditDefaultsOnly, Category = "Mouse Target")
	float RightMovementSpeed = 10.f;
	
	UPROPERTY(EditDefaultsOnly, Category = "Mouse Target")
	float ForwardMovementSpeed = 10.f;

	bool bIsCurrentlyShown;

	FVector2D CurrentInputVector;
};
