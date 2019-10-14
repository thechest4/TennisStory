// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "HalfCourt.generated.h"

UENUM(BlueprintType)
enum class ESnapPoint : uint8
{
	Mid,
	Left,
	Right
};

UCLASS()
class TENNISSTORY_API AHalfCourt : public AActor
{
	GENERATED_BODY()
	
public:	
	AHalfCourt();

	FVector2D GetCourtBounds2D();

	FVector GetPlayerServiceLocation() const
	{
		return PlayerServiceLocation->GetComponentLocation();
	}

	FVector GetBallServiceLocation() const
	{
		return BallServiceLocation->GetComponentLocation();
	}

	FVector GetSnapPointLocation(ESnapPoint SnapPoint);

	void ClampLocationToCourtBounds(FVector& Location);

protected:
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere, Category = "StartLocations")
	USceneComponent* PlayerServiceLocation;

	UPROPERTY(EditAnywhere, Category = "StartLocations")
	USceneComponent* BallServiceLocation;

	UPROPERTY(EditAnywhere, Category = "Target Snap Points")
	USceneComponent* MidSnapPoint;

	UPROPERTY(EditAnywhere, Category = "Target Snap Points")
	USceneComponent* RightSnapPoint;

	UPROPERTY(EditAnywhere, Category = "Target Snap Points")
	USceneComponent* LeftSnapPoint;

	UPROPERTY(EditAnywhere, Category = "Court Dimensions")
	float CourtLength = 1188.5f;

	UPROPERTY(EditAnywhere, Category = "Court Dimensions")
	float CourtWidth = 823.0f;

	UPROPERTY(EditAnywhere, Category = "Court Dimensions")
	float AlleyWidth = 274.0f; //AlleyWidth should be added to CourtWidth in doubles

	void CalculateCourtCorners();

	FVector2D LowerCorner;
	FVector2D UpperCorner;

#if WITH_EDITOR
	UPROPERTY(VisibleDefaultsOnly)
	class UBoxComponent* EditorCourtBounds;

	UPROPERTY()
	class UBillboardComponent* PlayerServiceIcon;

	UPROPERTY()
	class UBillboardComponent* BallServiceIcon;

	UPROPERTY()
	class UBillboardComponent* MidSnapPointIcon;

	UPROPERTY()
	class UBillboardComponent* RightSnapPointIcon;

	UPROPERTY()
	class UBillboardComponent* LeftSnapPointIcon;

	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
};
