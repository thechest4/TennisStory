
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "GameplayAbilities/Public/AbilitySystemInterface.h"
#include "GameplayTags.h"
#include "TennisStoryCharacter.generated.h"

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
};

UCLASS(config=Game)
class ATennisStoryCharacter : public ACharacter, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:
	ATennisStoryCharacter();

	virtual void Tick(float DeltaSeconds) override;

	class UAbilitySystemComponent* GetAbilitySystemComponent() const override
	{
		return AbilitySystemComp;
	};

	UPROPERTY(EditAnywhere)
	class UBallStrikingComponent* BallStrikingComp;

	void EnablePlayerTargeting();

	void FreezePlayerTarget();

	void DisablePlayerTargeting();

	void CacheCourtAimVector(FVector AimVector);

	FVector GetAimVector() const
	{
		return CachedAimVector;
	}

	FVector GetAimRightVector() const
	{
		return CachedAimRightVector;
	}

protected:
	virtual void BeginPlay() override;

	virtual void PossessedBy(AController* NewController) override;

	void MoveForward(float Value);

	void MoveRight(float Value);

	void MoveTargetForward(float Value);

	void MoveTargetRight(float Value);

	TWeakObjectPtr<class AHalfCourt> GetCourtToAimAt();

	// APawn interface
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	// End of APawn interface

	UPROPERTY(EditAnywhere, Category = "Tennis Racquet")
	FName RacquetAttachBone = TEXT("hand_r");

	UPROPERTY(EditAnywhere, Category = "Tennis Racquet")
	TSubclassOf<class ATennisRacquet> RacquetClass;

	UPROPERTY(Instanced)
	ATennisRacquet* RacquetActor;

	UPROPERTY(EditAnywhere)
	TSubclassOf<class APlayerTargetActor> TargetActorClass;

	UPROPERTY(Instanced)
	APlayerTargetActor* TargetActor;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Abilities")
	UAbilitySystemComponent* AbilitySystemComp;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Abilities")
	TArray<FGrantedAbilityInfo> AbilitiesToGive;

	FGameplayTagContainer TestTagContainer;

	UPROPERTY(EditAnywhere, Category = "Movement")
	float MoveSpeedWhileSwinging = 150.0f;

	float CachedMaxWalkSpeed;

	//These vectors are used to orient the character relative to the court it's aiming at
	//Set whenever a character is assigned to a court
	FVector CachedAimVector;
	FVector CachedAimRightVector;

	friend class UBallStrikingComponent;
};

