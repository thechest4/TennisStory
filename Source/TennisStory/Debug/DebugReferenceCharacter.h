// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Player/TennisStoryCharacter.h"
#include "HasContextMenu.h"
#include "DebugReferenceCharacter.generated.h"

class UHighlightableSkelMeshComponent;
class UWidgetComponent;

UCLASS()
class TENNISSTORY_API ADebugReferenceCharacter : public ATennisStoryCharacter, public IHasContextMenu
{
	GENERATED_BODY()
	
public:
	ADebugReferenceCharacter(const FObjectInitializer& ObjectInitializer);

	virtual void BeginPlay() override;

	virtual void ShowContextMenu() override;

	virtual void HideContextMenu() override;

protected:
	UPROPERTY(EditDefaultsOnly)
	UHighlightableSkelMeshComponent* HighlightSkelMesh;

	UPROPERTY(VisibleAnywhere)
	UWidgetComponent* ContextMenuComp;
};
