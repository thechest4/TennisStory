// Fill out your copyright notice in the Description page of Project Settings.

#include "TrajectoryDataProvider.h"

UTrajectoryDataProvider::UTrajectoryDataProvider()
{
	if (this == GetDefault<UTrajectoryDataProvider>())
	{
		//Locate Mapping DT
		static ConstructorHelpers::FObjectFinder<UDataTable> MappingDTFinder(TEXT("DataTable'/Game/Blueprints/Gameplay/Trajectory/SourceTrajectoryMapping_DT.SourceTrajectoryMapping_DT'"));
		SourceTrajectoryMappingDT = MappingDTFinder.Object;

		if (!MappingDTFinder.Succeeded())
		{
			GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("UTrajectoryDataProvider - Unable to find SourceTrajectoryMappingDT"));
		}

		//Locate Color Mapping DT
		static ConstructorHelpers::FObjectFinder<UDataTable> ColorMappingDTFinder(TEXT("DataTable'/Game/Blueprints/Gameplay/Trajectory/ShotTypeColorMapping_DT.ShotTypeColorMapping_DT'"));
		ShotTypeColorMappingDT = ColorMappingDTFinder.Object;

		if (!ColorMappingDTFinder.Succeeded())
		{
			GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("UTrajectoryDataProvider - Unable to find ShotTypeColorMappingDT"));
		}
	}
}

UDataTable* UTrajectoryDataProvider::GetTrajectoryTableByTag(FGameplayTag SourceTag) const
{
	ensureMsgf(SourceTrajectoryMappingDT, TEXT("SourceTrajectoryMappingDT was null"));

	for (auto Row : SourceTrajectoryMappingDT->GetRowMap())
	{
		FSourceTrajectoryMapping* SourceTrajMapping = reinterpret_cast<FSourceTrajectoryMapping*>(Row.Value);

		ensureMsgf(SourceTrajMapping, TEXT("Wrong row struct in table"));

		if (SourceTrajMapping->ShotSourceTag.MatchesTagExact(SourceTag))
		{
			return SourceTrajMapping->SourceDT;
		}
	}

	checkNoEntry();

	return nullptr;
}

