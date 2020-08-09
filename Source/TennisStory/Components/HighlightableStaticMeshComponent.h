// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/StaticMeshComponent.h"
#include "HighlightableStaticMeshComponent.generated.h"

UCLASS()
class TENNISSTORY_API UHighlightableStaticMeshComponent : public UStaticMeshComponent
{
	GENERATED_BODY()
	
public:
	void StartHighlight(UMaterialInterface* HighlightMat);

	void EndHighlight();

protected:
	UPROPERTY()
	TArray<UMaterialInterface*> CachedMats;
};
