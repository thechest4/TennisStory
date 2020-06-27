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
		return BallSpeed;
	}
	
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
	virtual UCurveFloat* GetTrajectoryCurve_Implementation() override
	{
		return TrajectoryCurve;
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

	FVector CurrentDiveDirection;

	UPROPERTY()
	TWeakObjectPtr<UAnimInstance> OwnerAnimInstance;

	UPROPERTY(EditDefaultsOnly, Category = "Dive Parameters")
	float DiveSpeed; //The base speed
	
	
	UPROPERTY(EditDefaultsOnly, Category = "Trajectory")
	UCurveFloat* TrajectoryCurve;
	
	UPROPERTY(EditDefaultsOnly, Category = "Trajectory")
	float MidpointAdditiveHeight = 150.f;
	
	UPROPERTY(EditDefaultsOnly, Category = "Trajectory")
	float TangentLength = 500.f;
	
	UPROPERTY(EditDefaultsOnly, Category = "Ball Speed")
	float BallSpeed = 1000.0f;
};
