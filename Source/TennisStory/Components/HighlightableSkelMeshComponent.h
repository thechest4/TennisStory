// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/SkeletalMeshComponent.h"
#include "../Debug/CursorMovable.h"
#include "HighlightableSkelMeshComponent.generated.h"

UCLASS()
class TENNISSTORY_API UHighlightableSkelMeshComponent : public USkeletalMeshComponent, public ICursorMovable
{
	GENERATED_BODY()
	
public:
	virtual void StartHighlight(UMaterialInterface* HighlightMat) override;

	virtual void EndHighlight() override;

	virtual bool IsMoveTypeAllowed(ECursorMoveType MoveType) { return AllowedMoveTypes.Contains(MoveType); }

	void SetAllowedMoveTypes(TArray<ECursorMoveType> NewAllowedMoveTypes) { AllowedMoveTypes = NewAllowedMoveTypes; }

protected:
	TArray<UMaterialInterface*> CachedMats;

	TArray<ECursorMoveType> AllowedMoveTypes;
};
