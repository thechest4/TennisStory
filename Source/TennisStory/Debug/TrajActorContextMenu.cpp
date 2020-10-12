// Fill out your copyright notice in the Description page of Project Settings.

#include "TrajActorContextMenu.h"
#include "TrajectoryTestActor.h"
#include <Engine/DataTable.h>

void UTrajActorContextMenu::SetTrajActorRef(ATrajectoryTestActor* TrajActor)
{
	TrajActorRef = TrajActor;

	if (TrajectoryData_Old)
	{
		FString ContextStr;
		TArray<FTrajectoryParams_Old*> TrajRows;
		TrajectoryData_Old->GetAllRows(ContextStr, TrajRows);

		if (TrajRows.Num() > 0)
		{
			TrajActorRef->SetTrajParamsOld(*TrajRows[0]);
		}
	}
	
	if (TrajectoryData_New)
	{
		FString ContextStr;
		TArray<FTrajectoryParams_New*> TrajRows;
		TrajectoryData_New->GetAllRows(ContextStr, TrajRows);

		if (TrajRows.Num() > 0)
		{
			TrajActorRef->SetTrajParamsNew(*TrajRows[0]);
		}
	}
}

ETrajectoryAlgorithm UTrajActorContextMenu::GetSelectedTrajectoryAlgorithm_Implementation()
{
	return ETrajectoryAlgorithm::New;
}
