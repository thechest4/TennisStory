// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "ServeAbility.generated.h"

class ATennisBall;
class ATennisStoryCharacter;
class UBallMovementComponent;

UENUM()
enum class EServeQuality : uint8
{
	Bad,
	Normal,
	Perfect,
	MAX	UMETA(Hidden)
};

UCLASS()
class TENNISSTORY_API UServeAbility : public UGameplayAbility
{
	GENERATED_BODY()
	
public:
	UServeAbility();

	virtual bool CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags = nullptr, const FGameplayTagContainer* TargetTags = nullptr, OUT FGameplayTagContainer* OptionalRelevantTags = nullptr) const;

	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* OwnerInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;

	UFUNCTION()
	void HandleServeMontageBlendOut();
	
	void HandlePlayerHitServe(ATennisStoryCharacter* Player);

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Serve Properties")
	UAnimMontage* ServeMontage;
	
	UPROPERTY(EditDefaultsOnly, Category = "Serve Properties")
	TArray<float> OrderedSpeedMultipliers;
	
	UPROPERTY(EditDefaultsOnly, Category = "Serve Properties")
	float PerfectServeMargin = 0.15f;

	UPROPERTY(EditDefaultsOnly, Category = "Serve Properties")
	float BadServeMargin = 0.3f;

	FGameplayTag ShotSourceTag = FGameplayTag::RequestGameplayTag(TEXT("Shot.Source.Serve"));
	FGameplayTag FallbackShotTypeTag = FGameplayTag::RequestGameplayTag(TEXT("Shot.Type.Flat"));
	
	UPROPERTY(EditDefaultsOnly, Category = "Hit FX")
	TArray<UParticleSystem*> OrderedServeHitVFX;
	
	//Returns an index that is used to retrieve a serve speed and hit FX from the Ordered TArrays
	int EvaluateServeQuality(UBallMovementComponent* BallMovementComp);

	UPROPERTY()
	class UTS_AbilityTask_PlayMontageAndWait* CurrentMontageTask;

	bool bServeReleased;

	virtual void InputPressed(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo) override;

	UPROPERTY(EditDefaultsOnly, Category = "Hit SFX")
	TArray<USoundBase*> OrderedServeHitSFX;

	FDelegateHandle OnPlayerHitServeDelegateHandle;
};
