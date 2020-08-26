// Fill out your copyright notice in the Description page of Project Settings.

#include "HighlightableSkelMeshComponent.h"

void UHighlightableSkelMeshComponent::StartHighlight(UMaterialInterface* HighlightMat)
{
	if (!HighlightMat)
	{
		return;
	}

	if (!CachedMats.Num())
	{
		CachedMats = GetMaterials();
	}

	for (int i = 0; i < GetNumMaterials(); i++)
	{
		SetMaterial(i, HighlightMat);
	}
}

void UHighlightableSkelMeshComponent::EndHighlight()
{
	for (int i = 0; i < CachedMats.Num(); i++)
	{
		SetMaterial(i, CachedMats[i]);
	}
}

