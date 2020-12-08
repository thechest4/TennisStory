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
	BackMidSnapPoint = CreateDefaultSubobject<USceneComponent>(TEXT("Back Mid Target Snap Point"));
	BackMidSnapPoint->SetupAttachment(RootComponent);
	BackMidSnapPoint->SetRelativeLocation(FVector(-0.25f * CourtLength, 0.0f, 0.0f));

	BackRightSnapPoint = CreateDefaultSubobject<USceneComponent>(TEXT("Back Right Target Snap Point"));
	BackRightSnapPoint->SetupAttachment(RootComponent);
	BackRightSnapPoint->SetRelativeLocation(FVector(-0.25f * CourtLength, 0.25f * CourtWidth, 0.0f));

	BackLeftSnapPoint = CreateDefaultSubobject<USceneComponent>(TEXT("Back Left Target Snap Point"));
	BackLeftSnapPoint->SetupAttachment(RootComponent);
	BackLeftSnapPoint->SetRelativeLocation(FVector(-0.25f * CourtLength, -0.25f * CourtWidth, 0.0f));
	
	FrontMidSnapPoint = CreateDefaultSubobject<USceneComponent>(TEXT("Front Mid Target Snap Point"));
	FrontMidSnapPoint->SetupAttachment(RootComponent);
	FrontMidSnapPoint->SetRelativeLocation(FVector(0.1f * CourtLength, 0.0f, 0.0f));
	
	FrontRightSnapPoint = CreateDefaultSubobject<USceneComponent>(TEXT("Front Right Target Snap Point"));
	FrontRightSnapPoint->SetupAttachment(RootComponent);
	FrontRightSnapPoint->SetRelativeLocation(FVector(0.1f * CourtLength, 0.25f * CourtWidth, 0.0f));
	
	FrontLeftSnapPoint = CreateDefaultSubobject<USceneComponent>(TEXT("Front Left Target Snap Point"));
	FrontLeftSnapPoint->SetupAttachment(RootComponent);
	FrontLeftSnapPoint->SetRelativeLocation(FVector(0.1f * CourtLength, -0.25f * CourtWidth, 0.0f));
	//Target Snap Points End

	//Hardcoded SnapPoint navigation
	SnapPointArrayRows = 2;
	SnapPointArrayCols = 3;

	NavigableSnapPointArray = {	ESnapPoint::FrontLeft,	ESnapPoint::FrontMid,	ESnapPoint::FrontRight,
								ESnapPoint::BackLeft,	ESnapPoint::BackMid,	ESnapPoint::BackRight };

#if WITH_EDITORONLY_DATA
	EditorCourtBounds = CreateEditorOnlyDefaultSubobject<UBoxComponent>(TEXT("EditorCourtBounds"));
	if (EditorCourtBounds)
	{
		EditorCourtBounds->SetVisibility(true);
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

	BackMidSnapPointIcon = CreateEditorOnlyDefaultSubobject<UBillboardComponent>(TEXT("Back Mid Snap Point Icon"));
	if (BackMidSnapPointIcon)
	{
		BackMidSnapPointIcon->SetupAttachment(BackMidSnapPoint);
		BackMidSnapPointIcon->SetHiddenInGame(true);
		BackMidSnapPointIcon->SetRelativeLocation(FVector(0.0f, 0.0f, IconHeight));
		BackMidSnapPointIcon->SetEditorScale(IconEditorScale);

		if (TargetIcon.Succeeded())
		{
			BackMidSnapPointIcon->SetSprite(TargetIcon.Object);
		}
	}
	
	BackRightSnapPointIcon = CreateEditorOnlyDefaultSubobject<UBillboardComponent>(TEXT("Back Right Snap Point Icon"));
	if (BackRightSnapPointIcon)
	{
		BackRightSnapPointIcon->SetupAttachment(BackRightSnapPoint);
		BackRightSnapPointIcon->SetHiddenInGame(true);
		BackRightSnapPointIcon->SetRelativeLocation(FVector(0.0f, 0.0f, IconHeight));
		BackRightSnapPointIcon->SetEditorScale(IconEditorScale);
		
		if (TargetIcon.Succeeded())
		{
			BackRightSnapPointIcon->SetSprite(TargetIcon.Object);
		}
	}

	BackLeftSnapPointIcon = CreateEditorOnlyDefaultSubobject<UBillboardComponent>(TEXT("Back Left Snap Point Icon"));
	if (BackLeftSnapPointIcon)
	{
		BackLeftSnapPointIcon->SetupAttachment(BackLeftSnapPoint);
		BackLeftSnapPointIcon->SetHiddenInGame(true);
		BackLeftSnapPointIcon->SetRelativeLocation(FVector(0.0f, 0.0f, IconHeight));
		BackLeftSnapPointIcon->SetEditorScale(IconEditorScale);

		if (TargetIcon.Succeeded())
		{
			BackLeftSnapPointIcon->SetSprite(TargetIcon.Object);
		}
	}

	FrontMidSnapPointIcon = CreateEditorOnlyDefaultSubobject<UBillboardComponent>(TEXT("Front Mid Snap Point Icon"));
	if (FrontMidSnapPointIcon)
	{
		FrontMidSnapPointIcon->SetupAttachment(FrontMidSnapPoint);
		FrontMidSnapPointIcon->SetHiddenInGame(true);
		FrontMidSnapPointIcon->SetRelativeLocation(FVector(0.0f, 0.0f, IconHeight));
		FrontMidSnapPointIcon->SetEditorScale(IconEditorScale);

		if (TargetIcon.Succeeded())
		{
			FrontMidSnapPointIcon->SetSprite(TargetIcon.Object);
		}
	}
	
	FrontRightSnapPointIcon = CreateEditorOnlyDefaultSubobject<UBillboardComponent>(TEXT("Front Right Snap Point Icon"));
	if (FrontRightSnapPointIcon)
	{
		FrontRightSnapPointIcon->SetupAttachment(FrontRightSnapPoint);
		FrontRightSnapPointIcon->SetHiddenInGame(true);
		FrontRightSnapPointIcon->SetRelativeLocation(FVector(0.0f, 0.0f, IconHeight));
		FrontRightSnapPointIcon->SetEditorScale(IconEditorScale);
		
		if (TargetIcon.Succeeded())
		{
			FrontRightSnapPointIcon->SetSprite(TargetIcon.Object);
		}
	}

	FrontLeftSnapPointIcon = CreateEditorOnlyDefaultSubobject<UBillboardComponent>(TEXT("Front Left Snap Point Icon"));
	if (FrontLeftSnapPointIcon)
	{
		FrontLeftSnapPointIcon->SetupAttachment(FrontLeftSnapPoint);
		FrontLeftSnapPointIcon->SetHiddenInGame(true);
		FrontLeftSnapPointIcon->SetRelativeLocation(FVector(0.0f, 0.0f, IconHeight));
		FrontLeftSnapPointIcon->SetEditorScale(IconEditorScale);

		if (TargetIcon.Succeeded())
		{
			FrontLeftSnapPointIcon->SetSprite(TargetIcon.Object);
		}
	}
#endif
}

FVector AHalfCourt::GetNextSnapPointLocation(ESnapPoint& CurrentSnapPoint, FVector AimVector, int HorzInput, int VertInput)
{
	//Figure out if the Control Rotation (AimRightVector) is the same as this court
	FVector AimRightVector = FVector::CrossProduct(FVector::UpVector, AimVector);
	FVector CourtRightVector = GetActorRightVector();
	float DotProd = FVector::DotProduct(CourtRightVector, AimRightVector);

	//If rotation are the same, invert the forward axis
	//This is assuming that the function call is from the player is on the far court
	if (DotProd > 0.f && VertInput != 0)
	{
		VertInput = (VertInput == -1) ? 1 : -1;
	}
	else if (DotProd < 0.f && HorzInput != 0) //If rotations are different, invert the right axis.  This is assuming the player is on the near court
	{
		HorzInput = (HorzInput == -1) ? 1 : -1;
	}

	ESnapPoint NewSnapPoint = CurrentSnapPoint;
	int CurrentRow, CurrentCol;
	if (TryGetIndicesForNavigableSnapPoint(NewSnapPoint, CurrentRow, CurrentCol))
	{
		//If the input is straight up or straight down, we want to aim at one of the mid snap points... so the logic breaks a little bit
		if (HorzInput == 0 && VertInput != 0)
		{
			CurrentCol = 1;
			CurrentRow = (VertInput == 1) ? 1 : 0;
		}
		else
		{
			if (HorzInput != 0)
			{
				CurrentCol = FMath::Clamp(CurrentCol + HorzInput, 0, SnapPointArrayCols - 1);
			}
			
			if (VertInput != 0)
			{
				CurrentRow = FMath::Clamp(CurrentRow + VertInput, 0, SnapPointArrayRows - 1);
			}
		}

		NewSnapPoint = NavigableSnapPointArray[CurrentRow * SnapPointArrayCols + CurrentCol];

		FVector SnapPointLocation = GetNextSnapPointLocation(NewSnapPoint);

		CurrentSnapPoint = NewSnapPoint;
		return SnapPointLocation;
	}

	return FVector::ZeroVector;
}

FVector AHalfCourt::GetNextSnapPointLocation(ESnapPoint SnapPoint)
{
	switch (SnapPoint)
	{
		case ESnapPoint::ServiceAd:
		case ESnapPoint::ServiceDeuce:
		{
			float XCoord = 0.2f * CourtLength;
			float YCoord = 0.25f * CourtWidth;
			YCoord = (SnapPoint == ESnapPoint::ServiceDeuce) ? YCoord : -1.f * YCoord;

			FVector ServiceBoxSnapPointLocation = GetActorTransform().TransformPosition(FVector(XCoord, YCoord, 0.f));
		
			return ServiceBoxSnapPointLocation;
		}
		case ESnapPoint::FrontLeft:
		{
			return FrontLeftSnapPoint->GetComponentLocation();
		}
		case ESnapPoint::FrontMid:
		{
			return FrontMidSnapPoint->GetComponentLocation();
		}
		case ESnapPoint::FrontRight:
		{
			return FrontRightSnapPoint->GetComponentLocation();
		}
		case ESnapPoint::BackLeft:
		{
			return BackLeftSnapPoint->GetComponentLocation();
		}
		case ESnapPoint::BackMid:
		{
			return BackMidSnapPoint->GetComponentLocation();
		}
		case ESnapPoint::BackRight:
		{
			return BackRightSnapPoint->GetComponentLocation();
		}
		default:
		{
			checkNoEntry()

			return FVector::ZeroVector;
		}
	}
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

void AHalfCourt::GetServiceClampLocations(EServiceSide ServiceSide, FVector& MidCourtLocation, FVector& SideCourtLocation)
{
	MidCourtLocation = GetActorLocation() - GetActorForwardVector() * CourtLength * 0.535f;
	SideCourtLocation = GetActorRightVector() * CourtWidth * 0.5f;

	SideCourtLocation = (ServiceSide == EServiceSide::Deuce) ? MidCourtLocation + SideCourtLocation : MidCourtLocation - SideCourtLocation;
}

bool AHalfCourt::IsLocationInFrontHalfOfCourt(FVector Location)
{
	if (!IsLocationInBounds(Location, 0.f, EBoundsContext::FullCourt))
	{
		return false;
	}

	FVector RelativeLocation = Location - GetActorLocation();

	//If we're right on the court location
	if (!RelativeLocation.Size())
	{
		return true;
	}

	RelativeLocation.Normalize();

	float DotProd = FVector::DotProduct(RelativeLocation, GetActorForwardVector());

	if (DotProd >= 0.f)
	{
		return true;
	}

	return false;
}

bool AHalfCourt::IsLocationInFrontQuarterOfCourt(FVector Location)
{
	if (!IsLocationInBounds(Location, 0.f, EBoundsContext::FullCourt))
	{
		return false;
	}

	FVector FrontQuarterRefLocation = GetActorLocation() + GetActorForwardVector() * CourtLength * 0.25f;

	FVector RelativeLocation = Location - FrontQuarterRefLocation;

	//If we're right on the court location
	if (!RelativeLocation.Size())
	{
		return true;
	}

	RelativeLocation.Normalize();

	float DotProd = FVector::DotProduct(RelativeLocation, GetActorForwardVector());

	if (DotProd >= 0.f)
	{
		return true;
	}

	return false;
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

	BackMidSnapPoint->SetRelativeLocation(FVector(-0.25f * CourtLength, 0.0f, 0.0f));
	BackRightSnapPoint->SetRelativeLocation(FVector(-0.25f * CourtLength, 0.25f * CourtWidth, 0.0f));
	BackLeftSnapPoint->SetRelativeLocation(FVector(-0.25f * CourtLength, -0.25f * CourtWidth, 0.0f));

	FrontMidSnapPoint->SetRelativeLocation(FVector(0.1f * CourtLength, 0.0f, 0.0f));
	FrontRightSnapPoint->SetRelativeLocation(FVector(0.1f * CourtLength, 0.25f * CourtWidth, 0.0f));
	FrontLeftSnapPoint->SetRelativeLocation(FVector(0.1f * CourtLength, -0.25f * CourtWidth, 0.0f));
}

bool AHalfCourt::TryGetIndicesForNavigableSnapPoint(ESnapPoint SnapPoint, int& Row, int& Col)
{
	for (int i = 0; i < NavigableSnapPointArray.Num(); i++)
	{
		if (NavigableSnapPointArray[i] == SnapPoint)
		{
			Row = i / SnapPointArrayCols;
			Col = i % SnapPointArrayCols;

			return true;
		}
	}

	return false;
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

	FVector BackMidSnapPointLocation = BackMidSnapPoint->GetRelativeTransform().GetLocation();
	BackMidSnapPoint->SetRelativeLocation(FVector(BackMidSnapPointLocation.X, BackMidSnapPointLocation.Y, 0.0f));

	FVector BackRightSnapPointLocation = BackRightSnapPoint->GetRelativeTransform().GetLocation();
	BackRightSnapPoint->SetRelativeLocation(FVector(BackRightSnapPointLocation.X, BackRightSnapPointLocation.Y, 0.0f));

	FVector BackLeftSnapPointLocation = BackLeftSnapPoint->GetRelativeTransform().GetLocation();
	BackLeftSnapPoint->SetRelativeLocation(FVector(BackLeftSnapPointLocation.X, BackLeftSnapPointLocation.Y, 0.0f));

	FVector FrontMidSnapPointLocation = FrontMidSnapPoint->GetRelativeTransform().GetLocation();
	FrontMidSnapPoint->SetRelativeLocation(FVector(FrontMidSnapPointLocation.X, FrontMidSnapPointLocation.Y, 0.0f));

	FVector FrontRightSnapPointLocation = FrontRightSnapPoint->GetRelativeTransform().GetLocation();
	FrontRightSnapPoint->SetRelativeLocation(FVector(FrontRightSnapPointLocation.X, FrontRightSnapPointLocation.Y, 0.0f));

	FVector FrontLeftSnapPointLocation = FrontLeftSnapPoint->GetRelativeTransform().GetLocation();
	FrontLeftSnapPoint->SetRelativeLocation(FVector(FrontLeftSnapPointLocation.X, FrontLeftSnapPointLocation.Y, 0.0f));

	Super::PostEditChangeProperty(PropertyChangedEvent);
}
#endif

