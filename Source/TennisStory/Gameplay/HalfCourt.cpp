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

	DeucePlayerServiceLocation = CreateDefaultSubobject<USceneComponent>(TEXT("Deuce Player Service Location"));
	DeucePlayerServiceLocation->SetupAttachment(RootComponent);
	DeucePlayerServiceLocation->SetRelativeLocation(FVector(-0.6f * CourtLength, 0.25f * CourtWidth, 0.0f));
	
	AdPlayerServiceLocation = CreateDefaultSubobject<USceneComponent>(TEXT("Ad Player Service Location"));
	AdPlayerServiceLocation->SetupAttachment(RootComponent);
	AdPlayerServiceLocation->SetRelativeLocation(FVector(-0.6f * CourtLength, -0.25f * CourtWidth, 0.0f));

	DeuceBallServiceLocation = CreateDefaultSubobject<USceneComponent>(TEXT("Deuce Ball Service Location"));
	DeuceBallServiceLocation->SetupAttachment(RootComponent);
	DeuceBallServiceLocation->SetRelativeLocation(FVector(-0.5f * CourtLength, 0.25f * CourtWidth, 0.0f));
	
	AdBallServiceLocation = CreateDefaultSubobject<USceneComponent>(TEXT("Ad Ball Service Location"));
	AdBallServiceLocation->SetupAttachment(RootComponent);
	AdBallServiceLocation->SetRelativeLocation(FVector(-0.5f * CourtLength, -0.25f * CourtWidth, 0.0f));

	MidSnapPoint = CreateDefaultSubobject<USceneComponent>(TEXT("Mid Target Snap Point"));
	MidSnapPoint->SetupAttachment(RootComponent);
	MidSnapPoint->SetRelativeLocation(FVector(-0.25f * CourtLength, 0.0f, 0.0f));

	RightSnapPoint = CreateDefaultSubobject<USceneComponent>(TEXT("Right Target Snap Point"));
	RightSnapPoint->SetupAttachment(RootComponent);
	RightSnapPoint->SetRelativeLocation(FVector(-0.25f * CourtLength, 0.25f * CourtWidth, 0.0f));

	LeftSnapPoint = CreateDefaultSubobject<USceneComponent>(TEXT("Left Target Snap Point"));
	LeftSnapPoint->SetupAttachment(RootComponent);
	LeftSnapPoint->SetRelativeLocation(FVector(-0.25f * CourtLength, -0.25f * CourtWidth, 0.0f));

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

	static ConstructorHelpers::FObjectFinder<UTexture2D> TennisBallSprite(TEXT("/Game/Art/Icons/game-icons-dot-net/tennis-ball"));

	DeuceBallServiceIcon = CreateEditorOnlyDefaultSubobject<UBillboardComponent>(TEXT("Deuce Ball Service Icon"));
	if (DeuceBallServiceIcon)
	{
		DeuceBallServiceIcon->SetupAttachment(DeuceBallServiceLocation);
		DeuceBallServiceIcon->SetHiddenInGame(true);
		DeuceBallServiceIcon->SetRelativeLocation(FVector(0.0f, 0.0f, IconHeight));
		DeuceBallServiceIcon->SetEditorScale(IconEditorScale);

		if (TennisBallSprite.Succeeded())
		{
			DeuceBallServiceIcon->SetSprite(TennisBallSprite.Object);
		}
	}
	
	AdBallServiceIcon = CreateEditorOnlyDefaultSubobject<UBillboardComponent>(TEXT("Ad Ball Service Icon"));
	if (AdBallServiceIcon)
	{
		AdBallServiceIcon->SetupAttachment(AdBallServiceLocation);
		AdBallServiceIcon->SetHiddenInGame(true);
		AdBallServiceIcon->SetRelativeLocation(FVector(0.0f, 0.0f, IconHeight));
		AdBallServiceIcon->SetEditorScale(IconEditorScale);

		if (TennisBallSprite.Succeeded())
		{
			AdBallServiceIcon->SetSprite(TennisBallSprite.Object);
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
	if (SnapPoint == ESnapPoint::Mid)
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

bool AHalfCourt::IsLocationInBounds(FVector& Location)
{
	if (Location.X < LowerCorner.X ||
		Location.X > UpperCorner.X ||
		Location.Y < LowerCorner.Y ||
		Location.Y > UpperCorner.Y)
	{
		return false;
	}

	return true;
}

void AHalfCourt::ClampLocationToCourtBounds(FVector& Location)
{
	if (Location.X < LowerCorner.X)
	{
		Location.X = LowerCorner.X;
	}
	else if (Location.X > UpperCorner.X)
	{
		Location.X = UpperCorner.X;
	}

	if (Location.Y < LowerCorner.Y)
	{
		Location.Y = LowerCorner.Y;
	}
	else if (Location.Y > UpperCorner.Y)
	{
		Location.Y = UpperCorner.Y;
	}
}

void AHalfCourt::BeginPlay()
{
	Super::BeginPlay();
	
	CalculateCourtCorners();
}

void AHalfCourt::CalculateCourtCorners()
{
	float XOffset = CourtLength / 2.0f;
	float YOffset = CourtWidth / 2.0f;

	FVector ActorLocation = GetActorLocation();
	LowerCorner = FVector2D(ActorLocation.X - XOffset, ActorLocation.Y - YOffset);
	UpperCorner = FVector2D(ActorLocation.X + XOffset, ActorLocation.Y + YOffset);
}

#if WITH_EDITORONLY_DATA
void AHalfCourt::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	FName PropertyName = (PropertyChangedEvent.Property) ? PropertyChangedEvent.Property->GetFName() : NAME_None;
	if (PropertyName == GET_MEMBER_NAME_CHECKED(AHalfCourt, CourtLength) || PropertyName == GET_MEMBER_NAME_CHECKED(AHalfCourt, CourtWidth))
	{
		EditorCourtBounds->SetBoxExtent(FVector(CourtLength / 2.0f, CourtWidth / 2.0f, 10.0f));
	}

	//Reset all court locations to be at ground level, in case the z coordinate was accidentally modified
	FVector DeucePlayerServiceRelativeLocation = DeucePlayerServiceLocation->GetRelativeTransform().GetLocation();
	DeucePlayerServiceLocation->SetRelativeLocation(FVector(DeucePlayerServiceRelativeLocation.X, DeucePlayerServiceRelativeLocation.Y, 0.0f));
	
	FVector AdPlayerServiceRelativeLocation = AdPlayerServiceLocation->GetRelativeTransform().GetLocation();
	AdPlayerServiceLocation->SetRelativeLocation(FVector(AdPlayerServiceRelativeLocation.X, AdPlayerServiceRelativeLocation.Y, 0.0f));

	FVector DeuceBallServiceRelativeLocation = DeuceBallServiceLocation->GetRelativeTransform().GetLocation();
	DeuceBallServiceLocation->SetRelativeLocation(FVector(DeuceBallServiceRelativeLocation.X, DeuceBallServiceRelativeLocation.Y, 0.0f));
	
	FVector AdBallServiceRelativeLocation = AdBallServiceLocation->GetRelativeTransform().GetLocation();
	AdBallServiceLocation->SetRelativeLocation(FVector(AdBallServiceRelativeLocation.X, AdBallServiceRelativeLocation.Y, 0.0f));

	FVector MidSnapPointLocation = MidSnapPoint->GetRelativeTransform().GetLocation();
	MidSnapPoint->SetRelativeLocation(FVector(MidSnapPointLocation.X, MidSnapPointLocation.Y, 0.0f));

	FVector RightSnapPointLocation = RightSnapPoint->GetRelativeTransform().GetLocation();
	RightSnapPoint->SetRelativeLocation(FVector(RightSnapPointLocation.X, RightSnapPointLocation.Y, 0.0f));

	FVector LeftSnapPointLocation = LeftSnapPoint->GetRelativeTransform().GetLocation();
	LeftSnapPoint->SetRelativeLocation(FVector(LeftSnapPointLocation.X, LeftSnapPointLocation.Y, 0.0f));

	Super::PostEditChangeProperty(PropertyChangedEvent);
}
#endif

