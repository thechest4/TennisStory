// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Gameplay/HalfCourt.h"
#include "PlayerTargetActor.generated.h"

UENUM()
enum class ETargetingContext : uint8
{
	None,
	Service,
	GroundStroke,
	Volley
};

UENUM()
enum class ETargetingMode : uint8
{
	Simple,
	Precise
};

USTRUCT()
struct FTargetSavedMove
{
	GENERATED_BODY()

public:
	FTargetSavedMove()
	{
		TranslationVector = FVector::ZeroVector;
		MoveTimestamp = 0.f;
	}
	
	FTargetSavedMove(FVector Translation, float Timestamp)
	{
		TranslationVector = Translation;
		MoveTimestamp = Timestamp;
	}

	UPROPERTY()
	FVector TranslationVector;
	
	UPROPERTY()
	float MoveTimestamp;
};

UCLASS()
class TENNISSTORY_API APlayerTargetActor : public AActor
{
	GENERATED_BODY()
	
public:	
	APlayerTargetActor();

	float const GetMoveSpeed() { return MoveSpeed; };

	void AddInputVector(FVector Direction, float Value);

	void Tick(float DeltaSeconds) override;

	ESnapPoint GetStartingSnapPointForTargetingContext(ETargetingContext Context);

	EBoundsContext GetCourtBoundsContextForTargetingContext(ETargetingContext TargetingContext);

	void ShowTargetOnCourt(TWeakObjectPtr<class AHalfCourt> CourtToAimAt, bool bShowTarget, ETargetingContext TargetingContext);

	void DisableTargetMovement();

	void HideTarget();

	FVector GetDesiredLocationOffset() const
	{
		return FVector(0.f, 0.f, 0.1f);
	}

	ETargetingMode GetTargetingMode() { return CurrentTargetingMode; }

	void SetTargetingMode(ETargetingMode NewMode) { CurrentTargetingMode = NewMode; }

	bool IsCurrentlyVisible() { return bCurrentlyVisible; }

	void SetMovementTarget(AActor* Target) { MovementTarget = Target; }

protected:
	virtual void BeginPlay() override;

	ETargetingContext CurrentTargetingContext;

	bool bCurrentlyVisible;
	bool bCurrentlyMovable;
	TWeakObjectPtr<AHalfCourt> CurrentTargetCourt;
	ESnapPoint LastSnapPoint;

	ETargetingMode CurrentTargetingMode;

	UPROPERTY(VisibleDefaultsOnly, Category = "Player Target")
	UStaticMeshComponent* TargetMesh;

	UPROPERTY(EditAnywhere, Category = "Player Target")
	float MoveSpeed = 3.0f;

	FVector CurrentInputVector = FVector::ZeroVector;

	FVector ConsumeCurrentInputVector();

	FVector GetOwnerControlRotationVector();

	AActor* MovementTarget;

	//Target movement replication
	UFUNCTION(Server, Unreliable, WithValidation)
	void Server_Move(const TArray<FTargetSavedMove>& SavedMoves);

	UFUNCTION(Client, Unreliable)
	void Client_PruneSavedMoves(float LastConsumedMoveTimestamp);

	TArray<FTargetSavedMove> Client_SavedMoves; //Moves saved by an autonomous proxy to send to the server.  Held until the server confirms which moves were consumed

	float Server_LastMoveTimestamp; //The last time the server performed a move, in server time.  Used for validating moves
	float Server_LastConsumedMoveTimestamp; //The timestamp of the last move that was consumed.  Returned to the client so that it can prune the SavedMoves array, but also used by the server to avoid repeating moves
	//End Target movement replication
};
