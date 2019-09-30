// Fill out your copyright notice in the Description page of Project Settings.


#include "HalfCourt.h"

#if WITH_EDITOR
#include "Components/BoxComponent.h"
#endif

AHalfCourt::AHalfCourt()
{
	PrimaryActorTick.bCanEverTick = false;

#if WITH_EDITOR
	EditorCourtBounds = CreateDefaultSubobject<UBoxComponent>(TEXT("EditorCourtBounds"));
	EditorCourtBounds->bVisible = true;
	EditorCourtBounds->bHiddenInGame = false;
	EditorCourtBounds->SetGenerateOverlapEvents(false);
	EditorCourtBounds->SetCollisionProfileName(TEXT("NoCollision"));
	EditorCourtBounds->SetBoxExtent(FVector(CourtLength / 2.0f, CourtWidth / 2.0f, 10.0f)); //10 height just to make the bounds visible
#endif
}

FVector2D AHalfCourt::GetCourtBounds2D()
{
	return FVector2D(CourtLength, CourtWidth);
}

void AHalfCourt::BeginPlay()
{
	Super::BeginPlay();
	
}

#if WITH_EDITOR
void AHalfCourt::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	FName PropertyName = (PropertyChangedEvent.Property) ? PropertyChangedEvent.Property->GetFName() : NAME_None;
	if (PropertyName == GET_MEMBER_NAME_CHECKED(AHalfCourt, CourtLength) || PropertyName == GET_MEMBER_NAME_CHECKED(AHalfCourt, CourtWidth))
	{
		EditorCourtBounds->SetBoxExtent(FVector(CourtLength / 2.0f, CourtWidth / 2.0f, 10.0f));
	}

	Super::PostEditChangeProperty(PropertyChangedEvent);
}
#endif

