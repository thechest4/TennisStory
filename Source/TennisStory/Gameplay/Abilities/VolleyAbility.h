// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "Gameplay/Abilities/GroundstrokeAbilityInterface.h"
#include "VolleyAbility.generated.h"

class ATennisBall;
class ATennisStoryCharacter;
class UTS_AbilityTask_PlayMontageAndWait;
class UAbilityTask_Tick;

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
		return (bIsHighVolley) ? MidpointAdditiveHeight_High : MidpointAdditiveHeight_Low;
	}
	
	UFUNCTION(BlueprintNativeEvent)
	float GetTangentLength();
	virtual float GetTangentLength_Implementation() override
	{
		return (bIsHighVolley) ? TangentLength_High : TangentLength_Low;
	}

	UFUNCTION(BlueprintNativeEvent)
	UCurveFloat* GetTrajectoryCurve();
	virtual UCurveFloat* GetTrajectoryCurve_Implementation() override;

	UFUNCTION(BlueprintNativeEvent)
	int GetShotQuality();
	virtual int GetShotQuality_Implementation() override;
	//IGroundstrokeAbilityInterface end

protected:
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
	
	UPROPERTY(EditDefaultsOnly, Category = "Trajectory | High Volley")
	UCurveFloat* TrajectoryCurve_High;
	
	UPROPERTY(EditDefaultsOnly, Category = "Trajectory | High Volley")
	float MidpointAdditiveHeight_High = 0.f;
	
	UPROPERTY(EditDefaultsOnly, Category = "Trajectory | High Volley")
	float TangentLength_High = 0.f;
	
	UPROPERTY(EditDefaultsOnly, Category = "Trajectory | Low Volley")
	UCurveFloat* TrajectoryCurve_Low;
	
	UPROPERTY(EditDefaultsOnly, Category = "Trajectory | Low Volley")
	float MidpointAdditiveHeight_Low = 120.f;
	
	UPROPERTY(EditDefaultsOnly, Category = "Trajectory | Low Volley")
	float TangentLength_Low = 150.f;

	UPROPERTY()
	UTS_AbilityTask_PlayMontageAndWait* CurrentMontageTask;

	UPROPERTY()
	UAbilityTask_Tick* CurrentTickingTask;

	bool bVolleyReleased;

	bool bIsHighVolley;

	EVolleyType CurrentVolleyType;

	virtual void InputReleased(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo) override;

	bool ShouldChooseForehand(ATennisBall* TennisBall, ATennisStoryCharacter* OwnerCharacter);

	UFUNCTION()
	void HandleBallHit();

	UFUNCTION()
	void HandleTaskTick();
};
