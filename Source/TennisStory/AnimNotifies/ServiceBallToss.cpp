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
		TennisBall->TossStartLocation = TennisBall->GetActorLocation();
		TennisBall->TossEndLocation = TennisBall->GetActorLocation() + FVector(0.f ,0.f, TossHeight);
		TennisBall->CurrentTossAlpha = 0.f;
		TennisBall->TotalTossDuration = TotalDuration;

		TennisBall->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
	}
}

void UServiceBallToss::NotifyTick(USkeletalMeshComponent * MeshComp, UAnimSequenceBase * Animation, float FrameDeltaTime)
{
	Super::NotifyTick(MeshComp, Animation, FrameDeltaTime);

	ATennisStoryGameState* TSGameState = Cast<ATennisStoryGameState>(MeshComp->GetWorld()->GetGameState());
	ATennisBall* TennisBall = (TSGameState) ? TSGameState->GetTennisBall().Get() : nullptr;
	if (TennisBall)
	{
		FVector NewBallLocation = FVector::ZeroVector;

		if (TennisBall->CurrentTossAlpha <= 1.f) //Toss goes up
		{
			NewBallLocation = FMath::InterpCircularOut(TennisBall->TossStartLocation, TennisBall->TossEndLocation, TennisBall->CurrentTossAlpha);
		}
		else  //Toss goes down
		{
			NewBallLocation = FMath::InterpCircularIn(TennisBall->TossEndLocation, TennisBall->TossStartLocation, TennisBall->CurrentTossAlpha - 1.f);
		}

		TennisBall->SetActorLocation(NewBallLocation);

		TennisBall->CurrentTossAlpha += FrameDeltaTime / (TennisBall->TotalTossDuration * 0.5f);
	}
}

void UServiceBallToss::NotifyEnd(USkeletalMeshComponent * MeshComp, UAnimSequenceBase * Animation)
{
	Super::NotifyEnd(MeshComp, Animation);

	//Handle interruption
	
	ATennisStoryGameState* TSGameState = Cast<ATennisStoryGameState>(MeshComp->GetWorld()->GetGameState());
	ATennisBall* TennisBall = (TSGameState) ? TSGameState->GetTennisBall().Get() : nullptr;
	if (TennisBall)
	{
		TennisBall->TossStartLocation = FVector::ZeroVector;
		TennisBall->TossEndLocation = FVector::ZeroVector;
		TennisBall->CurrentTossAlpha = 0.f;
		TennisBall->TotalTossDuration = 0.f;

		ATennisStoryCharacter* CurrentServingCharacter = TSGameState->GetServingCharacter().Get();
		if (CurrentServingCharacter)
		{
			TennisBall->AttachToComponent(CurrentServingCharacter->GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, ATennisStoryCharacter::BallAttachBone);
		}
	}
}
