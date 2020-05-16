// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "VolleyAbility.generated.h"

class ATennisBall;
class ATennisStoryCharacter;

UCLASS()
class TENNISSTORY_API UVolleyAbility : public UGameplayAbility
{
	GENERATED_BODY()
	
public:
	UVolleyAbility();

	virtual bool CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags = nullptr, const FGameplayTagContainer* TargetTags = nullptr, OUT FGameplayTagContainer* OptionalRelevantTags = nullptr) const;

	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* OwnerInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;

	UFUNCTION()
	void HandleVolleyMontageBlendOut();

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	UAnimMontage* ForehandMontage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	UAnimMontage* BackhandMontage;
	
	UPROPERTY(EditAnywhere)
	float BaseSpeedDuringAbility = 150.0f;

	UPROPERTY()
	class UTS_AbilityTask_PlayMontageAndWait* CurrentMontageTask;

	bool bVolleyReleased;

	virtual void InputReleased(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo) override;

	bool ShouldChooseForehand(ATennisBall* TennisBall, ATennisStoryCharacter* OwnerCharacter);
};
