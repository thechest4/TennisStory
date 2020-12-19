// Fill out your copyright notice in the Description page of Project Settings.

#include "RefCharContextMenu.h"

void URefCharContextMenu::PositionRefCharStrikeZone(EStrikeZoneLocation StrokeType)
{
	if (RefCharRef)
	{
		RefCharRef->PositionStrikeZone(StrokeType);
	}
}
