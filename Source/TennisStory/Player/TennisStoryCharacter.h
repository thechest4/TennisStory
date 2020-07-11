
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
class APlayerMouseTarget;

DECLARE_MULTICAST_DELEGATE_OneParam(FOnPlayerHitServeEvent, ATennisStoryCharacter*)

UENUM(BlueprintType)
enum class EGroundStrokeAbility :uint8
{
	Swing,
	Volley
};

UENUM(BlueprintType)
enum class EStrokeType :uint8
{
	Forehand,
	Backhand,
	Forehand_High,
	Backhand_High,
	Dive
};

UENUM(BlueprintType)
enum class EAbilityInput : uint8
{
	None UMETA(DisplayName = "None"),
	Swing UMETA(DisplayName = "Swing"),
	Dive UMETA(DisplayName = "Dive")
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
	
	bool bEnableRotationFix; //Controls the rotation fix for the autonomous proxy

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_SetActorTransform(FTransform NewTransform);
	
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

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_PlaySound(USoundBase* Sound, FVector Location);

	bool DoesSwingAbilityHavePermissionToActivate(const UGameplayAbility* AskingAbility);

	bool ShouldPerformForehand(ATennisBall* TennisBall);

	static const FName AXISNAME_MOVEFORWARD;
	static const FName AXISNAME_MOVERIGHT;

protected:
	virtual void BeginPlay() override;

	virtual void Tick(float DeltaSeconds) override;

	virtual void PostInitializeComponents() override;

	virtual void PossessedBy(AController* NewController) override;

	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	void MoveForward(float Value);

	void MoveRight(float Value);

	void AddTargetForwardInput(float Value);

	void AddTargetRightInput(float Value);
	
	void AddTargetSimpleForwardInput(float Value);

	void AddTargetSimpleRightInput(float Value);
	
	void AddTargetPreciseForwardInput(float Value);

	void AddTargetPreciseRightInput(float Value);
	
	void AddMouseAimForwardInput(float Value);

	void AddMouseAimRightInput(float Value);

	void SpawnMouseTargetActor();

	UFUNCTION()
	void HandleCharacterMovementUpdated(float DeltaSeconds, FVector OldLocation, FVector OldVelocity);

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

	UPROPERTY(Replicated)
	APlayerTargetActor* TargetActor;

	UPROPERTY(EditAnywhere)
	TSubclassOf<APlayerMouseTarget> MouseTargetClass;

	UPROPERTY()
	APlayerMouseTarget* MouseTarget;

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
	USceneComponent* StrikeZoneLocation_Forehand_High;
	
	UPROPERTY(EditDefaultsOnly, Category = "Swing Detection")
	USceneComponent* StrikeZoneLocation_Backhand;
	
	UPROPERTY(EditDefaultsOnly, Category = "Swing Detection")
	USceneComponent* StrikeZoneLocation_Backhand_High;
	
	UPROPERTY(EditDefaultsOnly, Category = "Swing Detection")
	USceneComponent* StrikeZoneLocation_Dive;
	
#if WITH_EDITORONLY_DATA
	UPROPERTY()
	UBillboardComponent* StrikeZoneIcon_Forehand;
	UPROPERTY()
	UBillboardComponent* StrikeZoneIcon_Forehand_High;

	UPROPERTY()
	UBillboardComponent* StrikeZoneIcon_Backhand;
	UPROPERTY()
	UBillboardComponent* StrikeZoneIcon_Backhand_High;
	
	UPROPERTY()
	UBillboardComponent* StrikeZoneIcon_Dive;
#endif

	//Team Color
	UPROPERTY(Transient, ReplicatedUsing = OnRep_TeamId)
	int TeamId = -1;

	UFUNCTION()
	void OnRep_TeamId();

	UPROPERTY(EditDefaultsOnly, Category = "Team Color")
	TArray<UMaterialInterface*> TeamColorMaterials;

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
	
	void PerformDive();

private:
	static FOnPlayerSpawnedEvent PlayerSpawnedEvent;
	FOnPlayerHitServeEvent PlayerHitServeEvent;

	friend class UBallStrikingComponent;
};

