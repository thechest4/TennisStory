// Fill out your copyright notice in the Description page of Project Settings.

#include "TrajActorContextMenu.h"
#include "TrajectoryTestActor.h"
#include <Engine/DataTable.h>

void UTrajActorContextMenu::SetTrajActorRef(ATrajectoryTestActor* TrajActor)
{
	TrajActorRef = TrajActor;
	
	if (TrajectoryData)
	{
		FString ContextStr;
		TArray<FTrajectoryParams*> TrajRows;
		TrajectoryData->GetAllRows(ContextStr, TrajRows);

		if (TrajRows.Num() > 0)
		{
			TrajActorRef->SetTrajParams(*TrajRows[0]);
		}
	}
}
