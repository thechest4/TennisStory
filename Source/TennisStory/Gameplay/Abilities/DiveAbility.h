// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "Gameplay/Abilities/BallStrikingAbility.h"
#include "GameFramework/RootMotionSource.h"
#include "DiveAbility.generated.h"

class UTS_AbilityTask_PlayMontageAndWait;
class UAbilityTask_ApplyDiveRootMotion;

USTRUCT()
struct FDiveAbilityTargetData : public FGameplayAbilityTargetData
{
	GENERATED_BODY()

public:
	FDiveAbilityTargetData()
		: DiveInputVector(FVector_NetQuantize::ZeroVector)
	{
	}
	
	virtual UScriptStruct* GetScriptStruct() const override
	{
		return FDiveAbilityTargetData::StaticStruct();
	}

	virtual FVector GetEndPoint() const override { return DiveInputVector; }

	UPROPERTY()
	FVector_NetQuantize DiveInputVector;

	bool NetSerialize(FArchive& Ar, class UPackageMap* Map, bool& bOutSuccess);
};

template<>
struct TStructOpsTypeTraits<FDiveAbilityTargetData> : public TStructOpsTypeTraitsBase2<FDiveAbilityTargetData>
{
	enum
	{
		WithNetSerializer = true	// For now this is REQUIRED for FGameplayAbilityTargetDataHandle net serialization to work
	};
};

USTRUCT()
struct FRootMotionSource_DiveMotion : public FRootMotionSource
{
	GENERATED_BODY()

public:
	FRootMotionSource_DiveMotion();

	UPROPERTY()
	FVector DiveDirection;

	UPROPERTY()
	float DiveDistance;

	UPROPERTY()
	UCurveFloat* PositionCurve;

	virtual FRootMotionSource* Clone() const override;

	virtual bool Matches(const FRootMotionSource* Other) const override;

	virtual void PrepareRootMotion(
		float SimulationTime, 
		float MovementTickTime,
		const ACharacter& Character, 
		const UCharacterMovementComponent& MoveComponent
		) override;

	virtual bool NetSerialize(FArchive& Ar, UPackageMap* Map, bool& bOutSuccess) override;

	virtual UScriptStruct* GetScriptStruct() const override;

	virtual FString ToSimpleString() const override;

	virtual void AddReferencedObjects(class FReferenceCollector& Collector) override;
};

template<>
struct TStructOpsTypeTraits<FRootMotionSource_DiveMotion> : public TStructOpsTypeTraitsBase2<FRootMotionSource_DiveMotion>
{
	enum
	{
		WithNetSerializer = true,
		WithCopy = true
	};
};

UCLASS()
class TENNISSTORY_API UDiveAbility : public UGameplayAbility, public IBallStrikingAbility
{
	GENERATED_BODY()
	
public:
	UDiveAbility();

	virtual bool CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags = nullptr, const FGameplayTagContainer* TargetTags = nullptr, OUT FGameplayTagContainer* OptionalRelevantTags = nullptr) const;

	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* OwnerInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;

	UFUNCTION()
	void HandleDiveMontageBlendOut();
	
	//IBallStrikingAbility implementation
	virtual FGameplayTag GetShotSourceTag() override
	{
		return FGameplayTag::RequestGameplayTag(TEXT("Shot.Source.Dive"));
	}

	virtual FGameplayTag GetFallbackShotTypeTag() override
	{
		return FallbackGameplayTag;
	}
	//IBallStrikingAbility end

protected:
	UPROPERTY(EditDefaultsOnly, Category = "Gameplay Tags")
	FGameplayTag FallbackGameplayTag = FGameplayTag::RequestGameplayTag(TEXT("Shot.Type.Topspin"));

	UPROPERTY(EditDefaultsOnly, Category = "Animations")
	UAnimMontage* DiveMontage;

	UPROPERTY()
	UTS_AbilityTask_PlayMontageAndWait* CurrentMontageTask;

	UPROPERTY()
	UAbilityTask_ApplyDiveRootMotion* CurrentRootMotionTask;

	UPROPERTY()
	TWeakObjectPtr<UAnimInstance> OwnerAnimInstance;
	
	UPROPERTY(EditDefaultsOnly, Category = "Dive Parameters")
	UCurveFloat* DivePositionCurve;

	UPROPERTY(EditDefaultsOnly, Category = "Dive Parameters")
	float DiveDistance;
	
	UPROPERTY(EditDefaultsOnly, Category = "Ball Speed")
	float BallSpeed = 1000.0f;

	FRotator CachedPrevRotation;
};
