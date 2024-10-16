
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "GameplayAbilities/Public/AbilitySystemInterface.h"
#include "GameplayAbilitySpec.h"
#include "GameplayTags.h"
#include "Components/SplineComponent.h"
#include "Gameplay/Ball/BallAimingFunctionLibrary.h"
#include "Player/PlayerTargetActor.h"
#include <GameplayTagContainer.h>
#include "TennisStoryCharacter.generated.h"

class ATennisBall;
class UBoxComponent;
class UDistanceIndicatorComponent;
class APlayerMouseTarget;
class UTrajectoryPreviewComponent;
class ICoreSwingAbility;

DECLARE_MULTICAST_DELEGATE_OneParam(FOnPlayerHitServeEvent, ATennisStoryCharacter*)

UENUM(BlueprintType)
enum class ECoreSwingAbility : uint8
{
	Swing,
	Volley,
	Smash
};

UENUM(BlueprintType)
enum class ESwingStance : uint8 
{
	Neutral,
	Forehand,
	Backhand
};

UENUM(BlueprintType)
enum class EStrikeZoneLocation : uint8
{
	Forehand,
	Backhand,
	Forehand_High,
	Backhand_High,
	Dive,
	Smash
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

	ATennisStoryCharacter(const FObjectInitializer& ObjectInitializer);

	class UAbilitySystemComponent* GetAbilitySystemComponent() const override
	{
		return AbilitySystemComp;
	};

	UPROPERTY(EditAnywhere)
	class UBallStrikingComponent* BallStrikingComp;

	void EnablePlayerTargeting(ETargetingContext TargetingContext, UObject* OverrideTrajSourceObj = nullptr);

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

	void PositionStrikeZone(EStrikeZoneLocation StrokeType);

	FVector GetStrikeZoneLocationForStroke(EStrikeZoneLocation StrokeType) const;

	//HACK(achester): This is a hack to try and fix a strange issue where SetActorRotation was not correctly working on the autonomous proxy character
	UPROPERTY(Transient, Replicated)
	FQuat ServerDesiredRotation;
	
	bool bEnableRotationFix; //Controls the rotation fix for the autonomous proxy.  DiveAbility disables this during the activation

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

	//If authority, calls the multicast version.  Otherwise does nothing
	void EnableSmashAbility();
	void DisableSmashAbility();

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_EnableSmashAbility();

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_DisableSmashAbility();

	//Resets any necessary state for beginning a point
	void ResetPlayStates();

	void CancelAllAbilities();

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_PlaySound(USoundBase* Sound, FVector Location);

	FGameplayAbilitySpecHandle GetHandleForAppropriateCoreSwingAbility(FGameplayTag& OutEventTag);

	ESwingStance CalculateNewSwingStance(ATennisBall* TennisBall);

	ESwingStance GetCurrentStance() { return CurrentSwingStance; }

	void SetCurrentStance(ESwingStance NewStance)
	{
		CurrentSwingStance = NewStance;
	}

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_ReleaseForgivingAbility(FGameplayAbilitySpecHandle AbilitySpecHandle);

	static const FName AXISNAME_MOVEFORWARD;
	static const FName AXISNAME_MOVERIGHT;

	static const FName STATETAG_SERVICE;
	static const FName STATETAG_ABILITIESLOCKED;
	static const FName STATETAG_SMASHENABLED;

	static const FName EVENTTAG_SWING;
	static const FName EVENTTAG_VOLLEY;
	static const FName EVENTTAG_SMASH;
	static const FName EVENTTAG_DIVE;

	static const FName TAG_CORESWING_CONTEXTCHANGE;

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

	//Desired Shot Type
	float LastShotTypeRequestTimestamp;
	float ShotTypeRequestThrottle = 0.2f; //Prevent changing shot type more than once every 200 ms

	UPROPERTY(EditAnywhere, Category = "Shot Type")
	UParticleSystem* ShotTypeChangeParticle;

	void RequestTopspinShot()	{ ChangeDesiredShotType(FGameplayTag::RequestGameplayTag(TEXT("Shot.Type.Topspin"))); }
	void RequestSliceShot()		{ ChangeDesiredShotType(FGameplayTag::RequestGameplayTag(TEXT("Shot.Type.Slice"))); }
	void RequestFlatShot()		{ ChangeDesiredShotType(FGameplayTag::RequestGameplayTag(TEXT("Shot.Type.Flat"))); }
	void RequestLobShot()		{ ChangeDesiredShotType(FGameplayTag::RequestGameplayTag(TEXT("Shot.Type.Lob"))); }

	void ChangeDesiredShotType(FGameplayTag DesiredShotTypeTag);

	UFUNCTION(Server, Reliable, WithValidation)
	void Server_ChangeDesiredShotType(FGameplayTag DesiredShotTypeTag);

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

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Aiming")
	UTrajectoryPreviewComponent* TrajectoryPreviewComp;

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

	UPROPERTY(EditDefaultsOnly, Category = "Swing Detection")
	USceneComponent* StrikeZoneLocation_Smash;
	
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

	UPROPERTY()
	UBillboardComponent* StrikeZoneIcon_Smash;
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
	
	void PerformCoreSwing();

	void ReleaseCoreSwing();

	UFUNCTION(Server, WithValidation, Reliable)
	void Server_ReleaseCoreSwing();

	void PerformDive();

	ESwingStance CurrentSwingStance;

	ICoreSwingAbility* GetActiveCoreSwingAbility();

private:
	static FOnPlayerSpawnedEvent PlayerSpawnedEvent;
	FOnPlayerHitServeEvent PlayerHitServeEvent;

	friend class UBallStrikingComponent;
};

