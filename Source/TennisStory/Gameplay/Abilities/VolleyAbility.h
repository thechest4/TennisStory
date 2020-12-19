// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "Gameplay/Abilities/GroundstrokeAbilityInterface.h"
#include "ForgivingAbilityInterface.h"
#include "VolleyAbility.generated.h"

class ATennisBall;
class ATennisStoryCharacter;
class UTS_AbilityTask_PlayMontageAndWait;
class UAbilityTask_Tick;

UCLASS()
class TENNISSTORY_API UVolleyAbility : public UGameplayAbility, public IGroundstrokeAbilityInterface, public IForgivingAbilityInterface
{
	GENERATED_BODY()
	
public:
	UVolleyAbility();

	virtual bool CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags = nullptr, const FGameplayTagContainer* TargetTags = nullptr, OUT FGameplayTagContainer* OptionalRelevantTags = nullptr) const;

	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* OwnerInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;

	UFUNCTION()
	void HandleVolleyMontageBlendOut();
	
	//IGroundstrokeAbilityInterface implementation
	virtual FGameplayTag GetShotSourceTag() override
	{
		return FGameplayTag::RequestGameplayTag(TEXT("Shot.Source.Volley"));
	}

	virtual FGameplayTag GetFallbackShotTypeTag() override
	{
		return FallbackGameplayTag;
	}
	//IGroundstrokeAbilityInterface end

	//IForgivingAbilityInterface implementation
	virtual void ReleaseForgiveness() override;
	//IForgivingAbilityInterface end

protected:
	UPROPERTY(EditDefaultsOnly, Category = "Gameplay Tags")
	FGameplayTag FallbackGameplayTag = FGameplayTag::RequestGameplayTag(TEXT("Shot.Type.Flat"));

	UPROPERTY(EditDefaultsOnly, Category = "Animations | High Volley")
	UAnimMontage* ForehandMontage_High;
	
	UPROPERTY(EditDefaultsOnly, Category = "Animations | Low Volley")
	UAnimMontage* ForehandMontage_Low;

	UPROPERTY(EditDefaultsOnly, Category = "Animations | High Volley")
	UAnimMontage* BackhandMontage_High;

	UPROPERTY(EditDefaultsOnly, Category = "Animations | Low Volley")
	UAnimMontage* BackhandMontage_Low;
	
	UPROPERTY(EditAnywhere, Category = "Movement Speed")
	float BaseSpeedDuringAbility = 150.0f;
	
	UPROPERTY(EditDefaultsOnly, Category = "Ball Speed | High Volley")
	float PassiveVolleySpeed_High = 1000.0f;
	
	UPROPERTY(EditDefaultsOnly, Category = "Ball Speed | High Volley")
	float ActiveVolleySpeed_High = 2000.0f;
	
	UPROPERTY(EditDefaultsOnly, Category = "Ball Speed | Low Volley")
	float PassiveVolleySpeed_Low = 800.0f;
	
	UPROPERTY(EditDefaultsOnly, Category = "Ball Speed | Low Volley")
	float ActiveVolleySpeed_Low = 1500.0f;

	UPROPERTY()
	UTS_AbilityTask_PlayMontageAndWait* CurrentMontageTask;

	UPROPERTY()
	UAbilityTask_Tick* CurrentTickingTask;

	bool bVolleyReleased;
	bool bCurrentShotIsHigh;

	virtual void InputReleased(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo) override;

	//returns true if Context has changed
	bool UpdateShotContext(ATennisBall* TennisBall, ATennisStoryCharacter* OwnerCharacter);

	UAnimMontage* GetVolleyMontage();

	void SetStrikeZonePosition(ATennisStoryCharacter* OwnerCharacter);

	UFUNCTION()
	void HandleBallHit();

	UFUNCTION()
	void HandleTaskTick(float DeltaTime);

	void HandleSwingForgivenessEnded();
};
