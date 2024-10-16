// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/StaticMeshComponent.h"
#include "../Debug/CursorMovable.h"
#include "HighlightableStaticMeshComponent.generated.h"

UCLASS()
class TENNISSTORY_API UHighlightableStaticMeshComponent : public UStaticMeshComponent, public ICursorMovable
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
