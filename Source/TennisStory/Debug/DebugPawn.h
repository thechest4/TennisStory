// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "DebugPawn.generated.h"

class UHighlightableStaticMeshComponent;

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
	UPROPERTY(EditAnywhere, Category = "Mouse Selection")
	UMaterialInterface* HighlightMat;

	UPROPERTY()
	APlayerController* PC;

	UPROPERTY()
	UHighlightableStaticMeshComponent* CurrentHighlightMesh;
};
