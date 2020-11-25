// Fill out your copyright notice in the Description page of Project Settings.

#include "TrajActorContextMenu.h"
#include "TrajectoryTestActor.h"
#include <Engine/DataTable.h>

void UTrajActorContextMenu::SetTrajActorRef(ATrajectoryTestActor* TrajActor)
{
	TrajActorRef = TrajActor;
	TrajActorRef->UpdateTrajParams(StartingSourceTag, FGameplayTagContainer::EmptyContainer, StartingShotType, StartingShotType);
}
