// Fill out your copyright notice in the Description page of Project Settings.

#include "TrajectoryDataProvider.h"
#include <Engine/DataTable.h>

UTrajectoryDataProvider::UTrajectoryDataProvider()
{
	if (this == GetDefault<UTrajectoryDataProvider>())
	{
		static ConstructorHelpers::FObjectFinder<UDataTable> DTFinder(TEXT("DataTable'/Game/Blueprints/Gameplay/Trajectory/TrajectoryParams_DT.TrajectoryParams_DT'"));
		TrajectoryParamsDT = DTFinder.Object;

		if (!DTFinder.Succeeded())
		{
			GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("UTrajectoryDataProvider - Unable to find the TrajectoryParams_DT"));
		}
	}
}

const UDataTable* UTrajectoryDataProvider::GetDefaultTrajectoryParamsDT()
{
	return GetDefault<UTrajectoryDataProvider>()->GetTrajectoryParamsDataTable();
}

