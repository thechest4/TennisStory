// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "TrajectoryDataProvider.generated.h"

class UDataTable;

UCLASS()
class TENNISSTORY_API UTrajectoryDataProvider : public UObject
{
	GENERATED_BODY()
	
public:
	UTrajectoryDataProvider();

	static const UDataTable* GetDefaultTrajectoryParamsDT();

	const UDataTable* GetTrajectoryParamsDataTable() const { return TrajectoryParamsDT; }

protected:
	UPROPERTY(EditDefaultsOnly)
	UDataTable* TrajectoryParamsDT;
};
