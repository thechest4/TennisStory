// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Gameplay/Ball/BallAimingFunctionLibrary.h"
#include "BallStrikingComponent.generated.h"

class UCurveFloat;
class USplineComponent;
class UGameplayAbility;
class ATennisStoryCharacter;
class ATennisRacquet;
class APlayerTargetActor;
class IGroundstrokeAbilityInterface;

DECLARE_EVENT(UBallStrikingComponent, FBallHitEvent)

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class TENNISSTORY_API UBallStrikingComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UBallStrikingComponent();

	void AllowBallStriking();

	void StopBallStriking();

	FBallHitEvent& OnBallHit() { return BallHitEvent; }

	void SetCurrentGroundstrokeAbility(UGameplayAbility* AbilityPtr);

protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	void HandleRacquetOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	
	UPROPERTY(EditDefaultsOnly, Category = "Hit FX")
	UParticleSystem* HitFX;

	UPROPERTY(EditDefaultsOnly, Category = "Hit SFX")
	TArray<USoundBase*> OrderedHitSFX;

	UPROPERTY()
	TScriptInterface<IGroundstrokeAbilityInterface> CurrentGroundstrokeAbility;

	//Cached Owner Pointers
	UPROPERTY()
	ATennisStoryCharacter* OwnerChar;
	
	UPROPERTY()
	ATennisRacquet* OwnerRacquet;

	UPROPERTY()
	APlayerTargetActor* OwnerTarget;

	bool bBallStrikingAllowed;

	FBallHitEvent BallHitEvent;

	friend class ATennisStoryCharacter;
};
