// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "Gameplay/Abilities/GroundstrokeAbilityInterface.h"
#include "DiveAbility.generated.h"

class UTS_AbilityTask_PlayMontageAndWait;
class UAbilityTask_Tick;

UCLASS()
class TENNISSTORY_API UDiveAbility : public UGameplayAbility, public IGroundstrokeAbilityInterface
{
	GENERATED_BODY()
	
public:
	UDiveAbility();

	virtual bool CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags = nullptr, const FGameplayTagContainer* TargetTags = nullptr, OUT FGameplayTagContainer* OptionalRelevantTags = nullptr) const;

	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* OwnerInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;
	
	UFUNCTION()
	void HandleDiveMontageBlendOut();
	
	//IGroundstrokeAbilityInterface implementation
	UFUNCTION(BlueprintNativeEvent)
	float CalculateBallSpeed();
	virtual float CalculateBallSpeed_Implementation() override
	{
		return 0.f;
	}
	
	UFUNCTION(BlueprintNativeEvent)
	float GetMidpointAdditiveHeight();
	virtual float GetMidpointAdditiveHeight_Implementation() override
	{
		float MidpointAdditiveHeight = 0.f;

		return MidpointAdditiveHeight;
	}
	
	UFUNCTION(BlueprintNativeEvent)
	float GetTangentLength();
	virtual float GetTangentLength_Implementation() override
	{
		float TangentLength = 0.f;

		return TangentLength;
	}

	UFUNCTION(BlueprintNativeEvent)
	UCurveFloat* GetTrajectoryCurve();
	virtual UCurveFloat* GetTrajectoryCurve_Implementation() override
	{
		return nullptr;
	}

	UFUNCTION(BlueprintNativeEvent)
	int GetShotQuality();
	virtual int GetShotQuality_Implementation() override
	{
		return 0;
	}
	//IGroundstrokeAbilityInterface end

protected:
	UPROPERTY(EditDefaultsOnly, Category = "Animations")
	UAnimMontage* DiveMontage;

	UPROPERTY()
	UTS_AbilityTask_PlayMontageAndWait* CurrentMontageTask;
	
	UPROPERTY()
	UAbilityTask_Tick* CurrentTickingTask;
	
	UFUNCTION()
	void HandleTaskTick(float DeltaTime);
};
