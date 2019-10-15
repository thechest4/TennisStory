// Fill out your copyright notice in the Description page of Project Settings.


#include "HalfCourt.h"
#include "TennisStoryGameMode.h"

#if WITH_EDITOR
#include "Components/BoxComponent.h"
#include "Components/BillboardComponent.h"
#include "Engine/Texture2D.h"
#endif

AHalfCourt::AHalfCourt()
{
	PrimaryActorTick.bCanEverTick = false;

	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Scene Root"));

	PlayerServiceLocation = CreateDefaultSubobject<USceneComponent>(TEXT("Player Service Location"));
	PlayerServiceLocation->SetupAttachment(RootComponent);
	PlayerServiceLocation->SetRelativeLocation(FVector(0.6f * CourtLength, -0.25f * CourtWidth, 0.0f));

	BallServiceLocation = CreateDefaultSubobject<USceneComponent>(TEXT("Ball Service Location"));
	BallServiceLocation->SetupAttachment(RootComponent);
	BallServiceLocation->SetRelativeLocation(FVector(0.5f * CourtLength, -0.25f * CourtWidth, 0.0f));

	MidSnapPoint = CreateDefaultSubobject<USceneComponent>(TEXT("Mid Target Snap Point"));
	MidSnapPoint->SetupAttachment(RootComponent);
	MidSnapPoint->SetRelativeLocation(FVector(0.25f * CourtLength, 0.0f, 0.0f));

	RightSnapPoint = CreateDefaultSubobject<USceneComponent>(TEXT("Right Target Snap Point"));
	RightSnapPoint->SetupAttachment(RootComponent);
	RightSnapPoint->SetRelativeLocation(FVector(0.25f * CourtLength, 0.25f * CourtWidth, 0.0f));

	LeftSnapPoint = CreateDefaultSubobject<USceneComponent>(TEXT("Left Target Snap Point"));
	LeftSnapPoint->SetupAttachment(RootComponent);
	LeftSnapPoint->SetRelativeLocation(FVector(0.25f * CourtLength, -0.25f * CourtWidth, 0.0f));

#if WITH_EDITOR
	EditorCourtBounds = CreateEditorOnlyDefaultSubobject<UBoxComponent>(TEXT("EditorCourtBounds"));
	EditorCourtBounds->bVisible = true;
	EditorCourtBounds->bHiddenInGame = false;
	EditorCourtBounds->SetGenerateOverlapEvents(false);
	EditorCourtBounds->SetCollisionProfileName(TEXT("NoCollision"));
	EditorCourtBounds->SetBoxExtent(FVector(CourtLength / 2.0f, CourtWidth / 2.0f, 10.0f)); //10 height just to make the bounds visible

	static const float IconHeight = 25.0f;
	static const float IconEditorScale = 0.5f;

	static ConstructorHelpers::FObjectFinder<UTexture2D> PlayerServiceSprite(TEXT("/Game/Art/Icons/game-icons-dot-net/throwing-ball"));

	PlayerServiceIcon = CreateEditorOnlyDefaultSubobject<UBillboardComponent>(TEXT("Player Service Icon"));
	PlayerServiceIcon->SetupAttachment(PlayerServiceLocation);
	PlayerServiceIcon->SetHiddenInGame(true);
	PlayerServiceIcon->SetRelativeLocation(FVector(0.0f, 0.0f, IconHeight));
	PlayerServiceIcon->SetEditorScale(IconEditorScale);

	if (PlayerServiceSprite.Succeeded())
	{
		PlayerServiceIcon->SetSprite(PlayerServiceSprite.Object);
	}

	static ConstructorHelpers::FObjectFinder<UTexture2D> TennisBallSprite(TEXT("/Game/Art/Icons/game-icons-dot-net/tennis-ball"));

	BallServiceIcon = CreateEditorOnlyDefaultSubobject<UBillboardComponent>(TEXT("Ball Service Icon"));
	BallServiceIcon->SetupAttachment(BallServiceLocation);
	BallServiceIcon->SetHiddenInGame(true);
	BallServiceIcon->SetRelativeLocation(FVector(0.0f, 0.0f, IconHeight));
	BallServiceIcon->SetEditorScale(IconEditorScale);

	if (TennisBallSprite.Succeeded())
	{
		BallServiceIcon->SetSprite(TennisBallSprite.Object);
	}

	MidSnapPointIcon = CreateEditorOnlyDefaultSubobject<UBillboardComponent>(TEXT("Mid Snap Point Icon"));
	MidSnapPointIcon->SetupAttachment(MidSnapPoint);
	MidSnapPointIcon->SetHiddenInGame(true);
	MidSnapPointIcon->SetRelativeLocation(FVector(0.0f, 0.0f, IconHeight));
	MidSnapPointIcon->SetEditorScale(IconEditorScale);
	
	RightSnapPointIcon = CreateEditorOnlyDefaultSubobject<UBillboardComponent>(TEXT("Right Snap Point Icon"));
	RightSnapPointIcon->SetupAttachment(RightSnapPoint);
	RightSnapPointIcon->SetHiddenInGame(true);
	RightSnapPointIcon->SetRelativeLocation(FVector(0.0f, 0.0f, IconHeight));
	RightSnapPointIcon->SetEditorScale(IconEditorScale);

	LeftSnapPointIcon = CreateEditorOnlyDefaultSubobject<UBillboardComponent>(TEXT("Left Snap Point Icon"));
	LeftSnapPointIcon->SetupAttachment(LeftSnapPoint);
	LeftSnapPointIcon->SetHiddenInGame(true);
	LeftSnapPointIcon->SetRelativeLocation(FVector(0.0f, 0.0f, IconHeight));
	LeftSnapPointIcon->SetEditorScale(IconEditorScale);

	static ConstructorHelpers::FObjectFinder<UTexture2D> TargetIcon(TEXT("/Game/Art/Icons/TargetIcon"));

	if (TargetIcon.Succeeded())
	{
		MidSnapPointIcon->SetSprite(TargetIcon.Object);
		RightSnapPointIcon->SetSprite(TargetIcon.Object);
		LeftSnapPointIcon->SetSprite(TargetIcon.Object);
	}
#endif
}

FVector AHalfCourt::GetSnapPointLocation(ESnapPoint SnapPoint)
{
	FVector LocationToReturn;

	switch (SnapPoint)
	{
		case ESnapPoint::Mid:
		{
			LocationToReturn = MidSnapPoint->GetComponentLocation();
			break;
		}
		case ESnapPoint::Right:
		{
			LocationToReturn = RightSnapPoint->GetComponentLocation();
			break;
		}
		case ESnapPoint::Left:
		{
			LocationToReturn = LeftSnapPoint->GetComponentLocation();
			break;
		}
		default:
		{
			LocationToReturn = MidSnapPoint->GetComponentLocation();
			break;
		}
	}

	return LocationToReturn;
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

#if WITH_EDITOR
void AHalfCourt::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	FName PropertyName = (PropertyChangedEvent.Property) ? PropertyChangedEvent.Property->GetFName() : NAME_None;
	if (PropertyName == GET_MEMBER_NAME_CHECKED(AHalfCourt, CourtLength) || PropertyName == GET_MEMBER_NAME_CHECKED(AHalfCourt, CourtWidth))
	{
		EditorCourtBounds->SetBoxExtent(FVector(CourtLength / 2.0f, CourtWidth / 2.0f, 10.0f));
	}

	//Reset all court locations to be at ground level, in case the z coordinate was accidentally modified
	FVector PlayerServiceRelativeLocation = PlayerServiceLocation->GetRelativeTransform().GetLocation();
	PlayerServiceLocation->SetRelativeLocation(FVector(PlayerServiceRelativeLocation.X, PlayerServiceRelativeLocation.Y, 0.0f));

	FVector BallServiceRelativeLocation = BallServiceLocation->GetRelativeTransform().GetLocation();
	BallServiceLocation->SetRelativeLocation(FVector(BallServiceRelativeLocation.X, BallServiceRelativeLocation.Y, 0.0f));

	FVector MidSnapPointLocation = MidSnapPoint->GetRelativeTransform().GetLocation();
	MidSnapPoint->SetRelativeLocation(FVector(MidSnapPointLocation.X, MidSnapPointLocation.Y, 0.0f));

	FVector RightSnapPointLocation = RightSnapPoint->GetRelativeTransform().GetLocation();
	RightSnapPoint->SetRelativeLocation(FVector(RightSnapPointLocation.X, RightSnapPointLocation.Y, 0.0f));

	FVector LeftSnapPointLocation = LeftSnapPoint->GetRelativeTransform().GetLocation();
	LeftSnapPoint->SetRelativeLocation(FVector(LeftSnapPointLocation.X, LeftSnapPointLocation.Y, 0.0f));

	Super::PostEditChangeProperty(PropertyChangedEvent);
}
#endif

