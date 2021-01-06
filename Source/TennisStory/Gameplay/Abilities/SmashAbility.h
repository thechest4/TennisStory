// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "Gameplay/Abilities/BallStrikingAbility.h"
#include "ForgivingAbilityInterface.h"
#include "CoreSwingAbility.h"
#include "SmashAbility.generated.h"

class ATennisBall;
class ATennisStoryCharacter;
class UTS_AbilityTask_PlayMontageAndWait;

UCLASS()
class TENNISSTORY_API USmashAbility : public UGameplayAbility, public IBallStrikingAbility, public IForgivingAbilityInterface, public ICoreSwingAbility
{
	GENERATED_BODY()
	
public:
	USmashAbility();

	virtual bool CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags = nullptr, const FGameplayTagContainer* TargetTags = nullptr, OUT FGameplayTagContainer* OptionalRelevantTags = nullptr) const;

	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* OwnerInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;

	UFUNCTION()
	void HandleSmashMontageBlendOut();

	//IBallStrikingAbility implementation
	virtual FGameplayTag GetShotSourceTag() override
	{
		return FGameplayTag::RequestGameplayTag(TEXT("Shot.Source.Smash"));
	}

	virtual FGameplayTag GetFallbackShotTypeTag() override
	{
		return FallbackGameplayTag;
	}
	//IBallStrikingAbility end

	//IForgivingAbilityInterface implementation
	virtual void ReleaseForgiveness() override;
	//IForgivingAbilityInterface end

	//ICoreSwingAbility implementation
	virtual void ReleaseSwing() override;

	virtual bool HasReleased() override { return bSmashReleased; }
	//ICoreSwingAbility end

protected:
	UPROPERTY(EditDefaultsOnly, Category = "Gameplay Tags")
	FGameplayTag FallbackGameplayTag = FGameplayTag::RequestGameplayTag(TEXT("Shot.Type.Flat"));

	UPROPERTY(EditDefaultsOnly, Category = "Animations")
	UAnimMontage* SmashMontage;

	UPROPERTY(EditAnywhere, Category = "Movement Speed")
	float BaseSpeedDuringAbility = 150.0f;

	UPROPERTY()
	UTS_AbilityTask_PlayMontageAndWait* CurrentMontageTask;

	bool bSmashReleased;

	void SetStrikeZonePosition(ATennisStoryCharacter* OwnerCharacter);

	void HandleSwingForgivenessEnded();
};
