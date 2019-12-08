// Fill out your copyright notice in the Description page of Project Settings.

#include "CommitServeNotify.h"
#include "Player/TennisStoryCharacter.h"

void UCommitServeNotify::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation)
{
	Super::Notify(MeshComp, Animation);

	ATennisStoryCharacter* TennisStoryChar = Cast<ATennisStoryCharacter>(MeshComp->GetOwner());
	if (TennisStoryChar)
	{
		TennisStoryChar->OnPlayerHitServe().Broadcast(TennisStoryChar);
	}
	else
	{
		GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Red, TEXT("UCommitServeNotify::Notify - Couldn't get a TennisStoryCharacter from MeshComp"));
	}
}
