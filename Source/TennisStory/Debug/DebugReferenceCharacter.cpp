// Fill out your copyright notice in the Description page of Project Settings.

#include "DebugReferenceCharacter.h"
#include "../Components/HighlightableSkelMeshComponent.h"
#include <UMG/Public/Components/WidgetComponent.h>
#include "RefCharContextMenu.h"

ADebugReferenceCharacter::ADebugReferenceCharacter(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer.DoNotCreateDefaultSubobject(TEXT("Mesh")).DoNotCreateDefaultSubobject(TEXT("CharacterMovement")).DoNotCreateDefaultSubobject(TEXT("AbilitySystemComp")).DoNotCreateDefaultSubobject(TEXT("BallStrikingComp")).DoNotCreateDefaultSubobject(TEXT("BallAimingSplineComp")))
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = false;

	RootComponent = HighlightSkelMesh = CreateDefaultSubobject<UHighlightableSkelMeshComponent>(TEXT("Highlight Mesh"));

	GetCapsuleComponent()->SetupAttachment(RootComponent);
	
	ContextMenuComp = CreateDefaultSubobject<UWidgetComponent>(TEXT("Context Menu Component"));
	ContextMenuComp->SetupAttachment(RootComponent);
	ContextMenuComp->SetWidgetSpace(EWidgetSpace::Screen);
}

void ADebugReferenceCharacter::BeginPlay()
{
	Super::BeginPlay();

	TArray<ECursorMoveType> AllowedMoveTypes = { ECursorMoveType::XY };
	HighlightSkelMesh->SetAllowedMoveTypes(AllowedMoveTypes);
	
	URefCharContextMenu* ContextMenu = Cast<URefCharContextMenu>(ContextMenuComp->GetUserWidgetObject());
	if (ContextMenu)
	{
		ContextMenu->SetVisibility(ESlateVisibility::Hidden);
		ContextMenu->SetRefCharRef(this);
	}

	PositionStrikeZone(EStrokeType::Forehand);
}

void ADebugReferenceCharacter::ShowContextMenu()
{
	UUserWidget* UserWidget = ContextMenuComp->GetUserWidgetObject();
	if (UserWidget)
	{
		UserWidget->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
	}
}

void ADebugReferenceCharacter::HideContextMenu()
{
	UUserWidget* UserWidget = ContextMenuComp->GetUserWidgetObject();
	if (UserWidget)
	{
		UserWidget->SetVisibility(ESlateVisibility::Hidden);
	}
}
