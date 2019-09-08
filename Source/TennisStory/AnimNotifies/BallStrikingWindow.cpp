// Fill out your copyright notice in the Description page of Project Settings.

#include "BallStrikingWindow.h"
#include "Player/TennisStoryCharacter.h"
#include "Player/Components/BallStrikingComponent.h"

void UBallStrikingWindow::NotifyBegin(USkeletalMeshComponent * MeshComp, UAnimSequenceBase * Animation, float TotalDuration)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration);

	ATennisStoryCharacter* OwnerChar = Cast<ATennisStoryCharacter>(MeshComp->GetOwner());
	UBallStrikingComponent* BallStrikingComp = (OwnerChar) ? OwnerChar->BallStrikingComp : nullptr;
	if (BallStrikingComp)
	{
		BallStrikingComp->AllowBallStriking();
	}
}

void UBallStrikingWindow::NotifyEnd(USkeletalMeshComponent * MeshComp, UAnimSequenceBase * Animation)
{
	Super::NotifyEnd(MeshComp, Animation);

	ATennisStoryCharacter* OwnerChar = Cast<ATennisStoryCharacter>(MeshComp->GetOwner());
	UBallStrikingComponent* BallStrikingComp = (OwnerChar) ? OwnerChar->BallStrikingComp : nullptr;
	if (BallStrikingComp)
	{
		BallStrikingComp->StopBallStriking();
	}
}
