// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "HalfCourt.generated.h"

class UBillboardComponent;

UENUM(BlueprintType)
enum class EBoundsContext : uint8
{
	FullCourt,
	ServiceDeuce,
	ServiceAd
};

UENUM(BlueprintType)
enum class EServiceSide : uint8
{
	Deuce,
	Ad
};

UENUM(BlueprintType)
enum class ECourtSide : uint8
{
	NearCourt,
	FarCourt
};

UENUM(BlueprintType)
enum class ESnapPoint : uint8
{
	ServiceDeuce,
	ServiceAd,
	BackMid,
	BackLeft,
	BackRight,
	FrontMid,
	FrontLeft,
	FrontRight
};

UCLASS()
class TENNISSTORY_API AHalfCourt : public AActor
{
	GENERATED_BODY()
	
public:	
	AHalfCourt();

	UFUNCTION(BlueprintCallable, Category = "Tennis Court")
	FTransform GetPlayerServiceTransform(EServiceSide ServiceSide) const
	{
		switch (ServiceSide)
		{
			case EServiceSide::Ad:
			{
				return AdPlayerServiceLocation->GetComponentTransform();
			}
			default:
			case EServiceSide::Deuce:
			{
				return DeucePlayerServiceLocation->GetComponentTransform();
			}
		}
	}

	UFUNCTION(BlueprintCallable, Category = "Tennis Court")
	FTransform GetReturnerTransform(EServiceSide ServiceSide) const
	{
		switch (ServiceSide)
		{
			case EServiceSide::Ad:
			{
				return AdReturnerLocation->GetComponentTransform();
			}
			default:
			case EServiceSide::Deuce:
			{
				return DeuceReturnerLocation->GetComponentTransform();
			}
		}
	}

	UFUNCTION(BlueprintCallable, Category = "Tennis Court")
	FTransform GetNetPlayerTransform(EServiceSide ServiceSide) const
	{
		switch (ServiceSide)
		{
			case EServiceSide::Ad:
			{
				return AdNetPlayerLocation->GetComponentTransform();
			}
			default:
			case EServiceSide::Deuce:
			{
				return DeuceNetPlayerLocation->GetComponentTransform();
			}
		}
	}

	FVector GetSnapPointLocation(FVector AimVector, ESnapPoint SnapPoint);

	bool IsLocationInBounds(FVector& Location, float BallRadius, EBoundsContext BoundsContext);

	void ClampLocationToCourtBounds(FVector& Location, EBoundsContext BoundsContext);

	ECourtSide GetCourtSide()
	{
		return CourtSide;
	}

	//The YAxisSign arg controls which corner will be returned, only checked for positive or negative
	FVector GetBackCornerWorldLocation(int YAxisSign);

	void GetServiceClampLocations(EServiceSide ServiceSide, FVector& MidCourtLocation, FVector& SideCourtLocation);

	bool IsLocationInFrontHalfOfCourt(FVector Location);

protected:
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere, Category = "Tennis Court")
	ECourtSide CourtSide;

	UPROPERTY(EditAnywhere, Category = "StartLocations")
	USceneComponent* DeucePlayerServiceLocation;

	UPROPERTY(EditAnywhere, Category = "StartLocations")
	USceneComponent* AdPlayerServiceLocation;
	
	UPROPERTY(EditAnywhere, Category = "StartLocations")
	USceneComponent* DeuceReturnerLocation;
	
	UPROPERTY(EditAnywhere, Category = "StartLocations")
	USceneComponent* DeuceNetPlayerLocation;

	UPROPERTY(EditAnywhere, Category = "StartLocations")
	USceneComponent* AdReturnerLocation;
	
	UPROPERTY(EditAnywhere, Category = "StartLocations")
	USceneComponent* AdNetPlayerLocation;

	UPROPERTY(EditAnywhere, Category = "Target Snap Points")
	USceneComponent* BackMidSnapPoint;

	UPROPERTY(EditAnywhere, Category = "Target Snap Points")
	USceneComponent* BackRightSnapPoint;

	UPROPERTY(EditAnywhere, Category = "Target Snap Points")
	USceneComponent* BackLeftSnapPoint;

	UPROPERTY(EditAnywhere, Category = "Target Snap Points")
	USceneComponent* FrontMidSnapPoint;

	UPROPERTY(EditAnywhere, Category = "Target Snap Points")
	USceneComponent* FrontRightSnapPoint;

	UPROPERTY(EditAnywhere, Category = "Target Snap Points")
	USceneComponent* FrontLeftSnapPoint;

	UPROPERTY(EditAnywhere, Category = "Court Dimensions")
	float CourtLength = 1188.5f;

	UPROPERTY(EditAnywhere, Category = "Court Dimensions")
	float CourtWidth = 823.0f;

	UPROPERTY(EditAnywhere, Category = "Court Dimensions")
	float AlleyWidth = 274.0f; //AlleyWidth should be added to CourtWidth in doubles

	void CalculateCourtCorners();

	FVector2D LowerCorner;
	FVector2D UpperCorner;
	
	FVector2D LowerCornerServiceDeuce;
	FVector2D UpperCornerServiceDeuce;
	
	FVector2D LowerCornerServiceAd;
	FVector2D UpperCornerServiceAd;

	void RecalculateCourtLocations();

#if WITH_EDITORONLY_DATA
	UPROPERTY(VisibleDefaultsOnly)
	class UBoxComponent* EditorCourtBounds;

	UPROPERTY()
	class UArrowComponent* CourtForwardArrow;

	UPROPERTY()
	UBillboardComponent* DeucePlayerServiceIcon;

	UPROPERTY()
	UBillboardComponent* AdPlayerServiceIcon;
	
	UPROPERTY()
	UBillboardComponent* DeuceReturnerIcon;
	
	UPROPERTY()
	UBillboardComponent* DeuceNetPlayerIcon;

	UPROPERTY()
	UBillboardComponent* AdReturnerIcon;
	
	UPROPERTY()
	UBillboardComponent* AdNetPlayerIcon;

	UPROPERTY()
	UBillboardComponent* BackMidSnapPointIcon;

	UPROPERTY()
	UBillboardComponent* BackRightSnapPointIcon;

	UPROPERTY()
	UBillboardComponent* BackLeftSnapPointIcon;

	UPROPERTY()
	UBillboardComponent* FrontMidSnapPointIcon;
						 
	UPROPERTY()			 
	UBillboardComponent* FrontRightSnapPointIcon;
						 
	UPROPERTY()			 
	UBillboardComponent* FrontLeftSnapPointIcon;

	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
};
