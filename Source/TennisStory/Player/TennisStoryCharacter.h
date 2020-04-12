
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "GameplayAbilities/Public/AbilitySystemInterface.h"
#include "GameplayAbilitySpec.h"
#include "GameplayTags.h"
#include "Components/SplineComponent.h"
#include "Gameplay/Ball/BallAimingFunctionLibrary.h"
#include "Player/PlayerTargetActor.h"
#include "TennisStoryCharacter.generated.h"

class ATennisBall;
class UBoxComponent;
class UDistanceIndicatorComponent;

DECLARE_MULTICAST_DELEGATE_OneParam(FOnPlayerHitServeEvent, ATennisStoryCharacter*)

UENUM(BlueprintType)
enum class EStrokeType :uint8
{
	Forehand,
	Backhand
};

UENUM(BlueprintType)
enum class EAbilityInput : uint8
{
	Swing UMETA(DisplayName = "Swing"),
};

USTRUCT(BlueprintType)
struct FGrantedAbilityInfo
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere)
	TSubclassOf<class UGameplayAbility> AbilityClass;

	UPROPERTY(EditAnywhere)
	EAbilityInput AbilityInput;

	FGameplayAbilitySpecHandle AbilitySpecHandle;
};

UCLASS(config=Game)
class ATennisStoryCharacter : public ACharacter, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:
	DECLARE_EVENT_OneParam(ATennisStoryCharacter, FOnPlayerSpawnedEvent, ATennisStoryCharacter*);
	static FOnPlayerSpawnedEvent& OnPlayerSpawned(){ return PlayerSpawnedEvent; }

	FOnPlayerHitServeEvent& OnPlayerHitServe(){ return PlayerHitServeEvent; }

	ATennisStoryCharacter();

	class UAbilitySystemComponent* GetAbilitySystemComponent() const override
	{
		return AbilitySystemComp;
	};

	UPROPERTY(EditAnywhere)
	class UBallStrikingComponent* BallStrikingComp;

	void EnablePlayerTargeting(ETargetingContext TargetingContext);

	void FreezePlayerTarget();

	void DisablePlayerTargeting();

	FVector GetCurrentTargetLocation(){ return TargetActor->GetActorLocation(); }

	void CacheCourtAimVector(FVector AimVector);

	FVector GetAimVector() const
	{
		return CachedAimVector;
	}

	FVector GetAimRightVector() const
	{
		return CachedAimRightVector;
	}

	UBoxComponent* GetStrikeZone()
	{
		return StrikeZone;
	}

	void PositionStrikeZone(EStrokeType StrokeType);

	FVector GetStrikeZoneLocationForStroke(EStrokeType StrokeType) const;

	//HACK(achester): This is a hack to try and fix a strange issue where SetActorRotation was not correctly working on the autonomous proxy character
	UPROPERTY(Transient, Replicated)
	FQuat ServerDesiredRotation;
	
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_SetActorTransform(FTransform NewTransform);

	void StartDistanceVisualizationToBall();

	void StopDistanceVisualization();
	
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_EnterServiceState();
	
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_ExitServiceState();

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_LockAbilities();
	
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_UnlockAbilities();

	bool HasBallAttached() const
	{
		return bHasBallAttached;
	}

	void AttachBallToPlayer(ATennisBall* TennisBall);

	void DetachBallFromPlayer(ATennisBall* TennisBall);

	const static FName BallAttachBone;

	//Movement Settings RPCs
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_LockMovement();
	
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_UnlockMovement();

	void ClampLocation(FVector MinLocation, FVector MaxLocation);

	void UnclampLocation();
	
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_ModifyBaseSpeed(float ModifiedBaseSpeed);
	
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_RestoreBaseSpeed();

	void CancelAllAbilities();

protected:
	virtual void BeginPlay() override;

	virtual void Tick(float DeltaSeconds) override;

	virtual void PostInitializeComponents() override;

	virtual void PossessedBy(AController* NewController) override;

	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	void MoveForward(float Value);

	void MoveRight(float Value);

	void MoveTargetForward(float Value);

	void MoveTargetRight(float Value);

	UFUNCTION()
	void HandleCharacterMovementUpdated(float DeltaSeconds, FVector OldLocation, FVector OldVelocity);

	UFUNCTION(Server, Reliable, WithValidation)
	void Server_CommitTargetPosition(FVector WorldLocation);

	// APawn interface
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	// End of APawn interface

	UPROPERTY(EditAnywhere, Category = "Tennis Racquet")
	FName RacquetAttachBone = TEXT("hand_r");

	UPROPERTY(EditAnywhere, Category = "Tennis Racquet")
	TSubclassOf<class ATennisRacquet> RacquetClass;

	UPROPERTY()
	ATennisRacquet* RacquetActor;

	UPROPERTY(EditAnywhere)
	TSubclassOf<APlayerTargetActor> TargetActorClass;

	UPROPERTY()
	APlayerTargetActor* TargetActor;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Abilities")
	UAbilitySystemComponent* AbilitySystemComp;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Abilities")
	TArray<FGrantedAbilityInfo> AbilitiesToGive;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Aiming")
	USplineComponent* BallAimingSplineComp;

	//These vectors are used to orient the character relative to the court it's aiming at
	//Set whenever a character is assigned to a court
	UPROPERTY(Transient, Replicated)
	FVector CachedAimVector;

	UPROPERTY(Transient, Replicated)
	FVector CachedAimRightVector;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Swing Detection")
	UBoxComponent* StrikeZone;

	UPROPERTY(EditDefaultsOnly, Category = "Swing Detection")
	USceneComponent* StrikeZoneLocation_Forehand;
	
	UPROPERTY(EditDefaultsOnly, Category = "Swing Detection")
	USceneComponent* StrikeZoneLocation_Backhand;
	
#if WITH_EDITORONLY_DATA
	UPROPERTY()
	UBillboardComponent* StrikeZoneIcon_Forehand;

	UPROPERTY()
	UBillboardComponent* StrikeZoneIcon_Backhand;
#endif

	//Team Color
	UPROPERTY(Transient, ReplicatedUsing = OnRep_TeamId)
	int TeamId = -1;

	UFUNCTION()
	void OnRep_TeamId();

	UPROPERTY(EditDefaultsOnly, Category = "Team Color")
	TArray<UMaterialInterface*> TeamColorMaterials;

	UPROPERTY(VisibleDefaultsOnly)
	UStaticMeshComponent* DistanceIndicatorRing;

	UPROPERTY(VisibleAnywhere)
	UDistanceIndicatorComponent* DistanceIndicatorComp;

	UPROPERTY(Replicated)
	bool bHasBallAttached;

	//Movement Settings
	float CurrentBaseMovementSpeed;
	float DefaultBaseMovementSpeed;

	UPROPERTY(Replicated)
	bool bIsLocationClamped;
	
	UPROPERTY(Replicated)
	FVector ClampLocation1;
	
	UPROPERTY(Replicated)
	FVector ClampLocation2;

private:
	static FOnPlayerSpawnedEvent PlayerSpawnedEvent;
	FOnPlayerHitServeEvent PlayerHitServeEvent;

	friend class UBallStrikingComponent;
};

