// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "HalfCourt.generated.h"

class UBillboardComponent;

UENUM(BlueprintType)
enum class ECourtSide : uint8
{
	NearCourt,
	FarCourt
};

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

	UFUNCTION(BlueprintCallable, Category = "Tennis Court")
	FTransform GetPlayerServiceTransform() const
	{
		return PlayerServiceLocation->GetComponentTransform();
	}

	UFUNCTION(BlueprintCallable, Category = "Tennis Court")
	FTransform GetBallServiceTransform() const
	{
		FTransform BallSpawnOffset = FTransform::Identity;
		BallSpawnOffset.SetTranslation(FVector(0.0f, 0.0f, 120.0f));

		return BallSpawnOffset * BallServiceLocation->GetComponentTransform();
	}

	FVector GetSnapPointLocation(FVector AimVector, ESnapPoint SnapPoint);

	void ClampLocationToCourtBounds(FVector& Location);

	ECourtSide GetCourtSide()
	{
		return CourtSide;
	}

protected:
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere, Category = "Tennis Court")
	ECourtSide CourtSide;

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
	class UArrowComponent* CourtForwardArrow;

	UPROPERTY()
	UBillboardComponent* PlayerServiceIcon;

	UPROPERTY()
	UBillboardComponent* BallServiceIcon;

	UPROPERTY()
	UBillboardComponent* MidSnapPointIcon;

	UPROPERTY()
	UBillboardComponent* RightSnapPointIcon;

	UPROPERTY()
	UBillboardComponent* LeftSnapPointIcon;

	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
};
