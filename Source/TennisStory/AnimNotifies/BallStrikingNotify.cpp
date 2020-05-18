// Fill out your copyright notice in the Description page of Project Settings.

#include "BallStrikingNotify.h"
#include "Player/TennisStoryCharacter.h"
#include "Player/Components/BallStrikingComponent.h"

void UBallStrikingNotify::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation)
{
	Super::Notify(MeshComp, Animation);

	ATennisStoryCharacter* OwnerChar = Cast<ATennisStoryCharacter>(MeshComp->GetOwner());
	UBallStrikingComponent* BallStrikingComp = (OwnerChar) ? OwnerChar->BallStrikingComp : nullptr;
	if (BallStrikingComp)
	{
		if (bAllowBallStriking)
		{
			BallStrikingComp->AllowBallStriking();
		}
		else
		{
			BallStrikingComp->StopBallStriking();
		}
	}
}
