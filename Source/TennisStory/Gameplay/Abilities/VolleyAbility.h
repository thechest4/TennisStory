// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "Gameplay/Abilities/GroundstrokeAbilityInterface.h"
#include "VolleyAbility.generated.h"

class ATennisBall;
class ATennisStoryCharacter;

UENUM()
enum class EVolleyType : uint8
{
	PassiveVolley,
	ActiveVolley
};

UCLASS()
class TENNISSTORY_API UVolleyAbility : public UGameplayAbility, public IGroundstrokeAbilityInterface
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
	UFUNCTION(BlueprintNativeEvent)
	float CalculateBallSpeed();
	virtual float CalculateBallSpeed_Implementation() override;
	
	UFUNCTION(BlueprintNativeEvent)
	float GetMidpointAdditiveHeight();
	virtual float GetMidpointAdditiveHeight_Implementation() override
	{
		return MidpointAdditiveHeight;
	}
	
	UFUNCTION(BlueprintNativeEvent)
	float GetTangentLength();
	virtual float GetTangentLength_Implementation() override
	{
		return TangentLength;
	}

	UFUNCTION(BlueprintNativeEvent)
	UCurveFloat* GetTrajectoryCurve();
	virtual UCurveFloat* GetTrajectoryCurve_Implementation() override;

	UFUNCTION(BlueprintNativeEvent)
	int GetShotQuality();
	virtual int GetShotQuality_Implementation() override;
	//IGroundstrokeAbilityInterface end

protected:
	UPROPERTY(EditDefaultsOnly, Category = "Animations")
	UAnimMontage* ForehandMontage;

	UPROPERTY(EditDefaultsOnly, Category = "Animations")
	UAnimMontage* BackhandMontage;
	
	UPROPERTY(EditAnywhere, Category = "Movement Speed")
	float BaseSpeedDuringAbility = 150.0f;
	
	UPROPERTY(EditDefaultsOnly, Category = "Ball Speed")
	float PassiveVolleySpeed = 1000.0f;
	
	UPROPERTY(EditDefaultsOnly, Category = "Ball Speed")
	float ActiveVolleySpeed = 2000.0f;
	
	UPROPERTY(EditDefaultsOnly, Category = "Trajectory")
	UCurveFloat* TrajectoryCurve;
	
	UPROPERTY(EditDefaultsOnly, Category = "Trajectory")
	float MidpointAdditiveHeight = 0.f;
	
	UPROPERTY(EditDefaultsOnly, Category = "Trajectory")
	float TangentLength = 0.f;

	UPROPERTY()
	class UTS_AbilityTask_PlayMontageAndWait* CurrentMontageTask;

	bool bVolleyReleased;

	EVolleyType CurrentVolleyType;

	virtual void InputReleased(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo) override;

	bool ShouldChooseForehand(ATennisBall* TennisBall, ATennisStoryCharacter* OwnerCharacter);

	UFUNCTION()
	void HandleBallHit();
};
