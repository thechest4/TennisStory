// Fill out your copyright notice in the Description page of Project Settings.

#include "HalfCourt.h"

#if WITH_EDITORONLY_DATA
#include "Components/BoxComponent.h"
#include "Components/BillboardComponent.h"
#include "Components/ArrowComponent.h"
#include "Engine/Texture2D.h"
#endif

AHalfCourt::AHalfCourt()
{
	PrimaryActorTick.bCanEverTick = false;

	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Scene Root"));

	//Service Locations
	DeucePlayerServiceLocation = CreateDefaultSubobject<USceneComponent>(TEXT("Deuce Player Service Location"));
	DeucePlayerServiceLocation->SetupAttachment(RootComponent);
	DeucePlayerServiceLocation->SetRelativeLocation(FVector(-0.6f * CourtLength, 0.25f * CourtWidth, 0.0f));
	
	AdPlayerServiceLocation = CreateDefaultSubobject<USceneComponent>(TEXT("Ad Player Service Location"));
	AdPlayerServiceLocation->SetupAttachment(RootComponent);
	AdPlayerServiceLocation->SetRelativeLocation(FVector(-0.6f * CourtLength, -0.25f * CourtWidth, 0.0f));
	//Service Locations End
	
	//Returner Locations
	DeuceReturnerLocation = CreateDefaultSubobject<USceneComponent>(TEXT("Deuce Returner Location"));
	DeuceReturnerLocation->SetupAttachment(RootComponent);
	DeuceReturnerLocation->SetRelativeLocation(FVector(-0.35f * CourtLength, 0.25f * CourtWidth, 0.0f));

	AdReturnerLocation = CreateDefaultSubobject<USceneComponent>(TEXT("Ad Returner Location"));
	AdReturnerLocation->SetupAttachment(RootComponent);
	AdReturnerLocation->SetRelativeLocation(FVector(-0.35f * CourtLength, -0.25f * CourtWidth, 0.0f));
	//Returner Locations End

	//Net Player Locations (For Doubles)
	DeuceNetPlayerLocation = CreateDefaultSubobject<USceneComponent>(TEXT("Deuce Player Net Location"));
	DeuceNetPlayerLocation->SetupAttachment(RootComponent);
	DeuceNetPlayerLocation->SetRelativeLocation(FVector(0.25f * CourtLength, 0.25f * CourtWidth, 0.0f));

	AdNetPlayerLocation = CreateDefaultSubobject<USceneComponent>(TEXT("Ad Player Net Location"));
	AdNetPlayerLocation->SetupAttachment(RootComponent);
	AdNetPlayerLocation->SetRelativeLocation(FVector(0.25f * CourtLength, -0.25f * CourtWidth, 0.0f));
	//Net Player Locations (For Doubles) End

	//Target Snap Points
	MidSnapPoint = CreateDefaultSubobject<USceneComponent>(TEXT("Mid Target Snap Point"));
	MidSnapPoint->SetupAttachment(RootComponent);
	MidSnapPoint->SetRelativeLocation(FVector(-0.25f * CourtLength, 0.0f, 0.0f));

	RightSnapPoint = CreateDefaultSubobject<USceneComponent>(TEXT("Right Target Snap Point"));
	RightSnapPoint->SetupAttachment(RootComponent);
	RightSnapPoint->SetRelativeLocation(FVector(-0.25f * CourtLength, 0.25f * CourtWidth, 0.0f));

	LeftSnapPoint = CreateDefaultSubobject<USceneComponent>(TEXT("Left Target Snap Point"));
	LeftSnapPoint->SetupAttachment(RootComponent);
	LeftSnapPoint->SetRelativeLocation(FVector(-0.25f * CourtLength, -0.25f * CourtWidth, 0.0f));
	//Target Snap Points End

#if WITH_EDITORONLY_DATA
	EditorCourtBounds = CreateEditorOnlyDefaultSubobject<UBoxComponent>(TEXT("EditorCourtBounds"));
	if (EditorCourtBounds)
	{
		EditorCourtBounds->bVisible = true;
		EditorCourtBounds->bHiddenInGame = false;
		EditorCourtBounds->SetGenerateOverlapEvents(false);
		EditorCourtBounds->SetCollisionProfileName(TEXT("NoCollision"));
		EditorCourtBounds->SetBoxExtent(FVector(CourtLength / 2.0f, CourtWidth / 2.0f, 10.0f)); //10 height just to make the bounds visible
	}

	CourtForwardArrow = CreateEditorOnlyDefaultSubobject<UArrowComponent>(TEXT("Court Forward Arrow"));
	if (CourtForwardArrow)
	{
		CourtForwardArrow->SetupAttachment(RootComponent);
		CourtForwardArrow->SetRelativeLocation(FVector(-0.5f * CourtLength, 0.0f, 0.0f));
	}

	static const float IconHeight = 25.0f;
	static const float IconEditorScale = 0.5f;

	static ConstructorHelpers::FObjectFinder<UTexture2D> PlayerServiceSprite(TEXT("/Game/Art/Icons/game-icons-dot-net/throwing-ball"));

	DeucePlayerServiceIcon = CreateEditorOnlyDefaultSubobject<UBillboardComponent>(TEXT("Deuce Player Service Icon"));
	if (DeucePlayerServiceIcon)
	{
		DeucePlayerServiceIcon->SetupAttachment(DeucePlayerServiceLocation);
		DeucePlayerServiceIcon->SetHiddenInGame(true);
		DeucePlayerServiceIcon->SetRelativeLocation(FVector(0.0f, 0.0f, IconHeight));
		DeucePlayerServiceIcon->SetEditorScale(IconEditorScale);

		if (PlayerServiceSprite.Succeeded())
		{
			DeucePlayerServiceIcon->SetSprite(PlayerServiceSprite.Object);
		}
	}
	
	AdPlayerServiceIcon = CreateEditorOnlyDefaultSubobject<UBillboardComponent>(TEXT("Ad Player Service Icon"));
	if (AdPlayerServiceIcon)
	{
		AdPlayerServiceIcon->SetupAttachment(AdPlayerServiceLocation);
		AdPlayerServiceIcon->SetHiddenInGame(true);
		AdPlayerServiceIcon->SetRelativeLocation(FVector(0.0f, 0.0f, IconHeight));
		AdPlayerServiceIcon->SetEditorScale(IconEditorScale);

		if (PlayerServiceSprite.Succeeded())
		{
			AdPlayerServiceIcon->SetSprite(PlayerServiceSprite.Object);
		}
	}

	static ConstructorHelpers::FObjectFinder<UTexture2D> ReturnerSprite(TEXT("/Game/Art/Icons/game-icons-dot-net/tennis-racket"));

	DeuceReturnerIcon = CreateEditorOnlyDefaultSubobject<UBillboardComponent>(TEXT("Deuce Returner Icon"));
	if (DeuceReturnerIcon)
	{
		DeuceReturnerIcon->SetupAttachment(DeuceReturnerLocation);
		DeuceReturnerIcon->SetHiddenInGame(true);
		DeuceReturnerIcon->SetRelativeLocation(FVector(0.0f, 0.0f, IconHeight));
		DeuceReturnerIcon->SetEditorScale(IconEditorScale);

		if (ReturnerSprite.Succeeded())
		{
			DeuceReturnerIcon->SetSprite(ReturnerSprite.Object);
		}
	}
	
	AdReturnerIcon = CreateEditorOnlyDefaultSubobject<UBillboardComponent>(TEXT("Ad Returner Icon"));
	if (AdReturnerIcon)
	{
		AdReturnerIcon->SetupAttachment(AdReturnerLocation);
		AdReturnerIcon->SetHiddenInGame(true);
		AdReturnerIcon->SetRelativeLocation(FVector(0.0f, 0.0f, IconHeight));
		AdReturnerIcon->SetEditorScale(IconEditorScale);

		if (ReturnerSprite.Succeeded())
		{
			AdReturnerIcon->SetSprite(ReturnerSprite.Object);
		}
	}
	
	static ConstructorHelpers::FObjectFinder<UTexture2D> NetPlayerSprite(TEXT("/Game/Art/Icons/game-icons-dot-net/tennis-racket"));

	DeuceNetPlayerIcon = CreateEditorOnlyDefaultSubobject<UBillboardComponent>(TEXT("Deuce Net Player Icon"));
	if (DeuceNetPlayerIcon)
	{
		DeuceNetPlayerIcon->SetupAttachment(DeuceNetPlayerLocation);
		DeuceNetPlayerIcon->SetHiddenInGame(true);
		DeuceNetPlayerIcon->SetRelativeLocation(FVector(0.0f, 0.0f, IconHeight));
		DeuceNetPlayerIcon->SetEditorScale(IconEditorScale);

		if (NetPlayerSprite.Succeeded())
		{
			DeuceNetPlayerIcon->SetSprite(NetPlayerSprite.Object);
		}
	}
	
	AdNetPlayerIcon = CreateEditorOnlyDefaultSubobject<UBillboardComponent>(TEXT("Ad Net Player Icon"));
	if (AdNetPlayerIcon)
	{
		AdNetPlayerIcon->SetupAttachment(AdNetPlayerLocation);
		AdNetPlayerIcon->SetHiddenInGame(true);
		AdNetPlayerIcon->SetRelativeLocation(FVector(0.0f, 0.0f, IconHeight));
		AdNetPlayerIcon->SetEditorScale(IconEditorScale);

		if (NetPlayerSprite.Succeeded())
		{
			AdNetPlayerIcon->SetSprite(NetPlayerSprite.Object);
		}
	}

	static ConstructorHelpers::FObjectFinder<UTexture2D> TargetIcon(TEXT("/Game/Art/Icons/TargetIcon"));

	MidSnapPointIcon = CreateEditorOnlyDefaultSubobject<UBillboardComponent>(TEXT("Mid Snap Point Icon"));
	if (MidSnapPointIcon)
	{
		MidSnapPointIcon->SetupAttachment(MidSnapPoint);
		MidSnapPointIcon->SetHiddenInGame(true);
		MidSnapPointIcon->SetRelativeLocation(FVector(0.0f, 0.0f, IconHeight));
		MidSnapPointIcon->SetEditorScale(IconEditorScale);

		if (TargetIcon.Succeeded())
		{
			MidSnapPointIcon->SetSprite(TargetIcon.Object);
		}
	}
	
	RightSnapPointIcon = CreateEditorOnlyDefaultSubobject<UBillboardComponent>(TEXT("Right Snap Point Icon"));
	if (RightSnapPointIcon)
	{
		RightSnapPointIcon->SetupAttachment(RightSnapPoint);
		RightSnapPointIcon->SetHiddenInGame(true);
		RightSnapPointIcon->SetRelativeLocation(FVector(0.0f, 0.0f, IconHeight));
		RightSnapPointIcon->SetEditorScale(IconEditorScale);
		
		if (TargetIcon.Succeeded())
		{
			RightSnapPointIcon->SetSprite(TargetIcon.Object);
		}
	}

	LeftSnapPointIcon = CreateEditorOnlyDefaultSubobject<UBillboardComponent>(TEXT("Left Snap Point Icon"));
	if (LeftSnapPointIcon)
	{
		LeftSnapPointIcon->SetupAttachment(LeftSnapPoint);
		LeftSnapPointIcon->SetHiddenInGame(true);
		LeftSnapPointIcon->SetRelativeLocation(FVector(0.0f, 0.0f, IconHeight));
		LeftSnapPointIcon->SetEditorScale(IconEditorScale);

		if (TargetIcon.Succeeded())
		{
			LeftSnapPointIcon->SetSprite(TargetIcon.Object);
		}
	}
#endif
}

FVector AHalfCourt::GetSnapPointLocation(FVector AimVector, ESnapPoint SnapPoint)
{
	if (SnapPoint == ESnapPoint::ServiceDeuce || SnapPoint == ESnapPoint::ServiceAd)
	{
		float XCoord = 0.2f * CourtLength;
		float YCoord = 0.25f * CourtWidth;
		YCoord = (SnapPoint == ESnapPoint::ServiceDeuce) ? YCoord : -1.f * YCoord;

		FVector ServiceBoxSnapPoint = GetActorTransform().TransformPosition(FVector(XCoord, YCoord, 0.f));
		
		return ServiceBoxSnapPoint;
	}
	else if (SnapPoint == ESnapPoint::Mid)
	{
		return MidSnapPoint->GetComponentLocation();
	}
	else
	{
		//Figure out if the Control Rotation (AimRightVector) is the same as this court
		FVector AimRightVector = FVector::CrossProduct(FVector::UpVector, AimVector);
		FVector CourtRightVector = GetActorRightVector();
		float DotProd = FVector::DotProduct(CourtRightVector, AimRightVector);

		//If rotation are the same, return what was requested.  Otherwise return the inverse of the request
		if (DotProd > 0.f)
		{
			return (SnapPoint == ESnapPoint::Right) ? RightSnapPoint->GetComponentLocation() : LeftSnapPoint->GetComponentLocation();
		}
		else if (DotProd < 0.f)
		{
			return (SnapPoint == ESnapPoint::Right) ? LeftSnapPoint->GetComponentLocation() : RightSnapPoint->GetComponentLocation();
		}
	}

	return FVector::ZeroVector;
}

bool AHalfCourt::IsLocationInBounds(FVector& Location, float BallRadius, EBoundsContext BoundsContext)
{
	FVector2D LowerCornerToUse;
	FVector2D UpperCornerToUse;

	switch (BoundsContext)
	{
		case EBoundsContext::ServiceDeuce:
		{
			LowerCornerToUse = LowerCornerServiceDeuce;
			UpperCornerToUse = UpperCornerServiceDeuce;
			break;
		}
		case EBoundsContext::ServiceAd:
		{
			LowerCornerToUse = LowerCornerServiceAd;
			UpperCornerToUse = UpperCornerServiceAd;
			break;
		}
		default:
		case EBoundsContext::FullCourt:
		{
			LowerCornerToUse = LowerCorner;
			UpperCornerToUse = UpperCorner;
			break;
		}
	}

	if (Location.X < LowerCornerToUse.X - BallRadius ||
		Location.X > UpperCornerToUse.X + BallRadius ||
		Location.Y < LowerCornerToUse.Y - BallRadius ||
		Location.Y > UpperCornerToUse.Y + BallRadius)
	{
		return false;
	}

	return true;
}

void AHalfCourt::ClampLocationToCourtBounds(FVector& Location, EBoundsContext BoundsContext)
{
	FVector2D LowerCornerToUse;
	FVector2D UpperCornerToUse;

	switch (BoundsContext)
	{
		case EBoundsContext::ServiceDeuce:
		{
			LowerCornerToUse = LowerCornerServiceDeuce;
			UpperCornerToUse = UpperCornerServiceDeuce;
			break;
		}
		case EBoundsContext::ServiceAd:
		{
			LowerCornerToUse = LowerCornerServiceAd;
			UpperCornerToUse = UpperCornerServiceAd;
			break;
		}
		default:
		case EBoundsContext::FullCourt:
		{
			LowerCornerToUse = LowerCorner;
			UpperCornerToUse = UpperCorner;
			break;
		}
	}

	if (Location.X < LowerCornerToUse.X)
	{
		Location.X = LowerCornerToUse.X;
	}
	else if (Location.X > UpperCornerToUse.X)
	{
		Location.X = UpperCornerToUse.X;
	}

	if (Location.Y < LowerCornerToUse.Y)
	{
		Location.Y = LowerCornerToUse.Y;
	}
	else if (Location.Y > UpperCornerToUse.Y)
	{
		Location.Y = UpperCornerToUse.Y;
	}
}

FVector AHalfCourt::GetBackCornerWorldLocation(int YAxisSign)
{
	int Sign = FMath::Sign(YAxisSign);

	//If we get passed 0 somehow, just default to 1
	if (!Sign)
	{
		Sign = 1;
	}

	return GetActorTransform().TransformPosition(FVector(-0.5f * CourtLength, Sign * 0.5f * CourtWidth, 0.0f));
}

void AHalfCourt::GetServiceClampLocations(EServiceSide ServiceSide, FVector& MinClamp, FVector& MaxClamp)
{
	FVector MidCourtPosition = GetActorLocation() - GetActorForwardVector() * CourtLength * 0.535f;
	FVector SideCourtPosition = GetActorRightVector() * CourtWidth * 0.5f;

	SideCourtPosition = (ServiceSide == EServiceSide::Deuce) ? MidCourtPosition + SideCourtPosition : MidCourtPosition - SideCourtPosition;
	
	//TODO(achester): this is kind of dumb and relies on the CourtSide property to be up to date and managed.  Will need to revisit if we want to give each player the near court on their screen
	if ((ServiceSide == EServiceSide::Deuce && CourtSide == ECourtSide::NearCourt) ||
	    (ServiceSide == EServiceSide::Ad && CourtSide == ECourtSide::FarCourt))
	{
		MinClamp = MidCourtPosition;
		MaxClamp = SideCourtPosition;
	}
	else if ((ServiceSide == EServiceSide::Ad && CourtSide == ECourtSide::NearCourt) ||
			 (ServiceSide == EServiceSide::Deuce && CourtSide == ECourtSide::FarCourt))
	{
		MinClamp = SideCourtPosition;
		MaxClamp = MidCourtPosition;
	}
}

void AHalfCourt::BeginPlay()
{
	Super::BeginPlay();
	
	CalculateCourtCorners();
}

void AHalfCourt::CalculateCourtCorners()
{
	float XOffset = CourtLength / 2.f;
	float YOffset = CourtWidth / 2.f;

	FVector ActorLocation = GetActorLocation();
	LowerCorner = FVector2D(ActorLocation.X - XOffset, ActorLocation.Y - YOffset);
	UpperCorner = FVector2D(ActorLocation.X + XOffset, ActorLocation.Y + YOffset);

	FVector ServiceDeuceCenter = GetActorTransform().TransformPosition(FVector(0.25f * CourtLength, 0.25f * CourtWidth, 0.f));
	LowerCornerServiceDeuce = FVector2D(ServiceDeuceCenter.X, ServiceDeuceCenter.Y) + FVector2D(-0.25f * CourtLength, -0.25f * CourtWidth);
	UpperCornerServiceDeuce = FVector2D(ServiceDeuceCenter.X, ServiceDeuceCenter.Y) + FVector2D(0.25f * CourtLength, 0.25f * CourtWidth);
	
	FVector ServiceAdCenter = GetActorTransform().TransformPosition(FVector(0.25f * CourtLength, -0.25f * CourtWidth, 0.f));
	LowerCornerServiceAd = FVector2D(ServiceAdCenter.X, ServiceAdCenter.Y) + FVector2D(-0.25f * CourtLength, -0.25f * CourtWidth);
	UpperCornerServiceAd = FVector2D(ServiceAdCenter.X, ServiceAdCenter.Y) + FVector2D(0.25f * CourtLength, 0.25f * CourtWidth);
}

void AHalfCourt::RecalculateCourtLocations()
{
	DeucePlayerServiceLocation->SetRelativeLocation(FVector(-0.6f * CourtLength, 0.25f * CourtWidth, 0.0f));
	AdPlayerServiceLocation->SetRelativeLocation(FVector(-0.6f * CourtLength, -0.25f * CourtWidth, 0.0f));
	
	DeuceReturnerLocation->SetRelativeLocation(FVector(-0.35f * CourtLength, 0.25f * CourtWidth, 0.0f));
	AdReturnerLocation->SetRelativeLocation(FVector(-0.35f * CourtLength, -0.25f * CourtWidth, 0.0f));
	
	DeuceNetPlayerLocation->SetRelativeLocation(FVector(0.25f * CourtLength, 0.25f * CourtWidth, 0.0f));
	AdNetPlayerLocation->SetRelativeLocation(FVector(0.25f * CourtLength, -0.25f * CourtWidth, 0.0f));

	MidSnapPoint->SetRelativeLocation(FVector(-0.25f * CourtLength, 0.0f, 0.0f));
	RightSnapPoint->SetRelativeLocation(FVector(-0.25f * CourtLength, 0.25f * CourtWidth, 0.0f));
	LeftSnapPoint->SetRelativeLocation(FVector(-0.25f * CourtLength, -0.25f * CourtWidth, 0.0f));
}

#if WITH_EDITORONLY_DATA
void AHalfCourt::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	FName PropertyName = (PropertyChangedEvent.Property) ? PropertyChangedEvent.Property->GetFName() : NAME_None;
	if (PropertyName == GET_MEMBER_NAME_CHECKED(AHalfCourt, CourtLength) || PropertyName == GET_MEMBER_NAME_CHECKED(AHalfCourt, CourtWidth))
	{
		EditorCourtBounds->SetBoxExtent(FVector(CourtLength / 2.0f, CourtWidth / 2.0f, 10.0f));
		RecalculateCourtLocations();
	}

	//Reset all court locations to be at ground level, in case the z coordinate was accidentally modified
	FVector DeucePlayerServiceRelativeLocation = DeucePlayerServiceLocation->GetRelativeTransform().GetLocation();
	DeucePlayerServiceLocation->SetRelativeLocation(FVector(DeucePlayerServiceRelativeLocation.X, DeucePlayerServiceRelativeLocation.Y, 0.0f));
	
	FVector AdPlayerServiceRelativeLocation = AdPlayerServiceLocation->GetRelativeTransform().GetLocation();
	AdPlayerServiceLocation->SetRelativeLocation(FVector(AdPlayerServiceRelativeLocation.X, AdPlayerServiceRelativeLocation.Y, 0.0f));
	
	FVector DeuceReturnerRelativeLocation = DeuceReturnerLocation->GetRelativeTransform().GetLocation();
	DeuceReturnerLocation->SetRelativeLocation(FVector(DeuceReturnerRelativeLocation.X, DeuceReturnerRelativeLocation.Y, 0.0f));
	
	FVector AdReturnerRelativeLocation = AdReturnerLocation->GetRelativeTransform().GetLocation();
	AdReturnerLocation->SetRelativeLocation(FVector(AdReturnerRelativeLocation.X, AdReturnerRelativeLocation.Y, 0.0f));
	
	FVector DeuceNetPlayerRelativeLocation = DeuceNetPlayerLocation->GetRelativeTransform().GetLocation();
	DeuceNetPlayerLocation->SetRelativeLocation(FVector(DeuceNetPlayerRelativeLocation.X, DeuceNetPlayerRelativeLocation.Y, 0.0f));
	
	FVector AdNetPlayerRelativeLocation = AdNetPlayerLocation->GetRelativeTransform().GetLocation();
	AdNetPlayerLocation->SetRelativeLocation(FVector(AdNetPlayerRelativeLocation.X, AdNetPlayerRelativeLocation.Y, 0.0f));

	FVector MidSnapPointLocation = MidSnapPoint->GetRelativeTransform().GetLocation();
	MidSnapPoint->SetRelativeLocation(FVector(MidSnapPointLocation.X, MidSnapPointLocation.Y, 0.0f));

	FVector RightSnapPointLocation = RightSnapPoint->GetRelativeTransform().GetLocation();
	RightSnapPoint->SetRelativeLocation(FVector(RightSnapPointLocation.X, RightSnapPointLocation.Y, 0.0f));

	FVector LeftSnapPointLocation = LeftSnapPoint->GetRelativeTransform().GetLocation();
	LeftSnapPoint->SetRelativeLocation(FVector(LeftSnapPointLocation.X, LeftSnapPointLocation.Y, 0.0f));

	Super::PostEditChangeProperty(PropertyChangedEvent);
}
#endif

