// Fill out your copyright notice in the Description page of Project Settings.

#include "ServiceBallToss.h"
#include "TennisStoryGameState.h"
#include "Gameplay/Ball/TennisBall.h"
#include "Player/TennisStoryCharacter.h"

void UServiceBallToss::NotifyBegin(USkeletalMeshComponent * MeshComp, UAnimSequenceBase * Animation, float TotalDuration)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration);

	ATennisStoryGameState* TSGameState = Cast<ATennisStoryGameState>(MeshComp->GetWorld()->GetGameState());
	ATennisBall* TennisBall = (TSGameState) ? TSGameState->GetTennisBall().Get() : nullptr;
	if (TennisBall)
	{
		TennisBall->StartServiceToss(TossHeight, TotalDuration);
	}
}
