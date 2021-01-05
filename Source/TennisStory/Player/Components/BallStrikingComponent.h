// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Gameplay/Ball/BallAimingFunctionLibrary.h"
#include "BallStrikingComponent.generated.h"

class UCurveFloat;
class USplineComponent;
class UGameplayAbility;
class ATennisStoryCharacter;
class ATennisRacquet;
class APlayerTargetActor;
class IBallStrikingAbility;

DECLARE_EVENT(UBallStrikingComponent, FBallHitEvent)
DECLARE_EVENT(UBallStrikingComponent, FShotTagsChangedEvent)
DECLARE_EVENT(UBallStrikingComponent, FTimingForgivenessEndedEvent)

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class TENNISSTORY_API UBallStrikingComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UBallStrikingComponent();

	void AllowBallStriking();

	void StopBallStriking();

	FBallHitEvent& OnBallHit() { return BallHitEvent; }
	
	FShotTagsChangedEvent& OnShotTagsChanged() { return ShotTagsChangedEvent; }

	FTimingForgivenessEndedEvent& OnTimingForgivesnessEnded() { return TimingForgivenessEndedEvent; }

	void SetCurrentBallStrikingAbility(UGameplayAbility* AbilityPtr);

	void SetShotSourceAndFallbackTypeTags(FGameplayTag ShotSourceTag, FGameplayTag FallbackShotTypeTag) 
	{ 
		CurrentShotSourceTag = ShotSourceTag; 
		CurrentFallbackShotTypeTag = FallbackShotTypeTag; 

		ShotTagsChangedEvent.Broadcast();
	}

	void SetShotContextTags(FGameplayTagContainer ShotContextTags)
	{
		CurrentShotContextTags = ShotContextTags;

		ShotTagsChangedEvent.Broadcast();
	}

	FGameplayTagContainer GetShotContextTags()
	{
		return CurrentShotContextTags;
	}

	void SetDesiredShotTypeTag(FGameplayTag ShotTypeTag)
	{
		DesiredShotTypeTag = ShotTypeTag;

		ShotTagsChangedEvent.Broadcast();
	}

	FGameplayTag GetDesiredShotTypeTag() { return DesiredShotTypeTag; }

	void ResetAllShotTags(bool bResetContextTags = true, bool bResetDesiredTypeTag = true)
	{
		CurrentShotSourceTag = FGameplayTag::EmptyTag;
		CurrentFallbackShotTypeTag = FGameplayTag::EmptyTag;

		if (bResetContextTags)
		{
			CurrentShotContextTags = FGameplayTagContainer::EmptyContainer;
		}

		if (bResetDesiredTypeTag)
		{
			ServerDesiredShotType = FGameplayTag::EmptyTag;
			DesiredShotTypeTag = FGameplayTag::EmptyTag;
		}
	}

	bool ShouldWaitForTimingForgiveness(float AnimDelay);

protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	void HandleRacquetOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	
	UPROPERTY(EditDefaultsOnly, Category = "Hit FX")
	UParticleSystem* HitFX;

	UPROPERTY(EditDefaultsOnly, Category = "Hit SFX")
	TArray<USoundBase*> OrderedHitSFX;

	//Current Trajectory Identifiers
	UPROPERTY()
	UGameplayAbility* CurrentBallstrikingAbility;

	FGameplayTag CurrentShotSourceTag;
	FGameplayTag CurrentFallbackShotTypeTag;
	FGameplayTagContainer CurrentShotContextTags;
	FGameplayTag DesiredShotTypeTag;

	UPROPERTY(ReplicatedUsing = OnRep_ServerDesiredShotType)
	//Replicated DesiredShotType from Server to make sure our prediction is fixed if incorrect.  On Server this and DesiredShotTypeTag will be identical
	FGameplayTag ServerDesiredShotType;

	UFUNCTION()
	void OnRep_ServerDesiredShotType();

	//Cached Owner Pointers
	UPROPERTY()
	ATennisStoryCharacter* OwnerChar;
	
	UPROPERTY()
	ATennisRacquet* OwnerRacquet;

	UPROPERTY()
	APlayerTargetActor* OwnerTarget;

	bool bBallStrikingAllowed;

	//Timing Forgiveness
	UPROPERTY(EditDefaultsOnly, Category = "Timing Forgiveness")
	float ForgivenessThreshold = 0.5f; //The amount of time that player can swing in advance that we'll correct

	void EndTimingForgiveness() { TimingForgivenessEndedEvent.Broadcast(); }

	FBallHitEvent BallHitEvent;
	FShotTagsChangedEvent ShotTagsChangedEvent;
	FTimingForgivenessEndedEvent TimingForgivenessEndedEvent;

	friend class ATennisStoryCharacter;
	friend class UTrajectoryPreviewComponent;
};
