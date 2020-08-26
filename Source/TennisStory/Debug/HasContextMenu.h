// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "HasContextMenu.generated.h"

UINTERFACE(MinimalAPI)
class UHasContextMenu : public UInterface
{
	GENERATED_BODY()
};

class TENNISSTORY_API IHasContextMenu
{
	GENERATED_BODY()

public:
	virtual void ShowContextMenu(){}

	virtual void HideContextMenu(){}
};
