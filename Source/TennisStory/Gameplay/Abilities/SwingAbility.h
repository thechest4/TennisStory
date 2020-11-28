// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "Gameplay/Abilities/GroundstrokeAbilityInterface.h"
#include "SwingAbility.generated.h"

class ATennisBall;
class ATennisStoryCharacter;
class UTS_AbilityTask_PlayMontageAndWait;
class UAbilityTask_Tick;

UCLASS()
class TENNISSTORY_API USwingAbility : public UGameplayAbility, public IGroundstrokeAbilityInterface
{
	GENERATED_BODY()
	
public:
	USwingAbility();

	virtual bool CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags = nullptr, const FGameplayTagContainer* TargetTags = nullptr, OUT FGameplayTagContainer* OptionalRelevantTags = nullptr) const;

	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* OwnerInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;

	UFUNCTION()
	void HandleSwingMontageBlendOut();
	
	//IGroundstrokeAbilityInterface implementation
	virtual FGameplayTag GetShotSourceTag() override
	{
		return FGameplayTag::RequestGameplayTag(TEXT("Shot.Source.Swing"));
	}

	virtual FGameplayTag GetFallbackShotTypeTag() override
	{
		return FallbackGameplayTag;
	}

	virtual int GetShotQuality() override;

	virtual float GetSpeedMultiplier() override;
	//IGroundstrokeAbilityInterface end
protected:
	UPROPERTY(EditDefaultsOnly, Category = "Gameplay Tags")
	FGameplayTag FallbackGameplayTag = FGameplayTag::RequestGameplayTag(TEXT("Shot.Type.Topspin"));

	UPROPERTY(EditDefaultsOnly, Category = "Animations")
	UAnimMontage* ForehandMontage;

	UPROPERTY(EditDefaultsOnly, Category = "Animations")
	UAnimMontage* BackhandMontage;
	
	UPROPERTY(EditAnywhere, Category = "Movement Speed")
	float BaseSpeedDuringAbility = 150.0f;

	UPROPERTY()
	UTS_AbilityTask_PlayMontageAndWait* CurrentMontageTask;
	
	UPROPERTY()
	UAbilityTask_Tick* CurrentTickingTask;

	bool bSwingReleased;
	
	UPROPERTY(EditDefaultsOnly, Category = "Ball Speed")
	float MinSpeedMult = 1.f;

	UPROPERTY(EditDefaultsOnly, Category = "Ball Speed")
	float MaxSpeedMult = 2.f;
	
	UPROPERTY(EditDefaultsOnly, Category = "Ball Speed")
	float MaxChargeDuration = 3.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Hit SFX")
	float ChargeThresholdForMediumHitSFX = 0.3f;

	float LastChargeStartTime = 0.0f;
	float LastChargeEndTime = 0.0f;

	virtual void InputReleased(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo) override;

	bool bCurrentShotIsForehand;

	//returns true if Context has changed
	bool UpdateShotContext(ATennisBall* TennisBall, ATennisStoryCharacter* OwnerCharacter);

	UAnimMontage* GetSwingMontage();

	void SetStrikeZonePosition(ATennisStoryCharacter* OwnerCharacter);

	UFUNCTION()
	void HandleTaskTick(float DeltaTime);
};
