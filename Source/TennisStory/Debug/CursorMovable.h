// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "CursorMovable.generated.h"

UENUM()
enum class ECursorMoveType : uint8
{
	XY,
	Z
};

UINTERFACE(MinimalAPI)
class UCursorMovable : public UInterface
{
	GENERATED_BODY()
};

class TENNISSTORY_API ICursorMovable
{
	GENERATED_BODY()

public:
	virtual void StartHighlight(UMaterialInterface* HighlightMat){}

	virtual void EndHighlight(){}

	virtual bool IsMoveTypeAllowed(ECursorMoveType MoveType) { return true; }
};
