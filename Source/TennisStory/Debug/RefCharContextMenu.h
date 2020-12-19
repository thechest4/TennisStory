// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "DebugReferenceCharacter.h"
#include "RefCharContextMenu.generated.h"

UCLASS()
class TENNISSTORY_API URefCharContextMenu : public UUserWidget
{
	GENERATED_BODY()
	
public:
	void SetRefCharRef(ADebugReferenceCharacter* RefChar) { RefCharRef = RefChar; }

	UFUNCTION(BlueprintCallable)
	void PositionRefCharStrikeZone(EStrikeZoneLocation StrokeType);

protected:
	UPROPERTY(Transient)
	ADebugReferenceCharacter* RefCharRef;
};
