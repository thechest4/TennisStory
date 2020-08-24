// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "DebugPawn.generated.h"

class UHighlightableStaticMeshComponent;
class UCameraComponent;

UENUM()
enum class EHighlightInteractionType : uint8
{
	None,
	XY,
	Z
};

UENUM()
enum class EMouseDragType : uint8
{
	None,
	Camera,
	HighlightXY,
	HighlightZ
};

UCLASS()
class TENNISSTORY_API ADebugPawn : public APawn
{
	GENERATED_BODY()

public:
	ADebugPawn();

	virtual void BeginPlay() override;

	virtual void Tick(float DeltaTime) override;

	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

protected:
	UPROPERTY(EditAnywhere, Category = "DebugPawn")
	UMaterialInterface* HighlightMat;

	UPROPERTY(EditAnywhere, Category = "DebugPawn")
	float CameraRotationSpeed = 5.f;
	
	UPROPERTY(EditAnywhere, Category = "DebugPawn")
	float CameraMoveSpeed = 700.f;
	
	UPROPERTY(VisibleAnywhere)
	UCameraComponent* CameraComp;

	UPROPERTY()
	APlayerController* PC;

	UPROPERTY()
	UHighlightableStaticMeshComponent* CurrentHighlightMesh;

	EMouseDragType CurrentMouseDragType;

	bool bLockCurrentHighlightMesh;

	UPROPERTY(Transient)
	FVector2D CursorSelectionOffset;

	void StartLeftMouseDrag();

	void StopLeftMouseDrag();

	void StartRightMouseDrag();

	void StopRightMouseDrag();

	void MoveForward(float Value);
	
	void MoveRight(float Value);

	void CalculateSelectionOffset();
};
