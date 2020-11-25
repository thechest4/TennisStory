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
class IGroundstrokeAbilityInterface;

DECLARE_EVENT(UBallStrikingComponent, FBallHitEvent)
DECLARE_EVENT(UBallStrikingComponent, FShotTagsChangedEvent)

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

	void SetCurrentGroundstrokeAbility(UGameplayAbility* AbilityPtr);

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
			DesiredShotTypeTag = FGameplayTag::EmptyTag;
		}
	}

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
	TScriptInterface<IGroundstrokeAbilityInterface> CurrentGroundstrokeAbility;

	FGameplayTag CurrentShotSourceTag;
	FGameplayTag CurrentFallbackShotTypeTag;
	FGameplayTagContainer CurrentShotContextTags;
	FGameplayTag DesiredShotTypeTag;

	//Cached Owner Pointers
	UPROPERTY()
	ATennisStoryCharacter* OwnerChar;
	
	UPROPERTY()
	ATennisRacquet* OwnerRacquet;

	UPROPERTY()
	APlayerTargetActor* OwnerTarget;

	bool bBallStrikingAllowed;

	FBallHitEvent BallHitEvent;
	FShotTagsChangedEvent ShotTagsChangedEvent;

	friend class ATennisStoryCharacter;
	friend class UTrajectoryPreviewComponent;
};
