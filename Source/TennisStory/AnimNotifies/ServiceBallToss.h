// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "ServiceBallToss.generated.h"

UCLASS()
class TENNISSTORY_API UServiceBallToss : public UAnimNotifyState
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, Category = "Toss Parameters")
	float TossHeight = 200.f;

	virtual void NotifyBegin(USkeletalMeshComponent * MeshComp, UAnimSequenceBase * Animation, float TotalDuration) override;
};
