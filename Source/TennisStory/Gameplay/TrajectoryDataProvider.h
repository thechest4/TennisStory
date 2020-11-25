// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include <GameplayTagContainer.h>
#include <Engine/DataTable.h>
#include "TrajectoryDataProvider.generated.h"

USTRUCT(BlueprintType)
struct FSourceTrajectoryMapping : public FTableRowBase
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FGameplayTag ShotSourceTag;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	UDataTable* SourceDT;
};

UCLASS()
class TENNISSTORY_API UTrajectoryDataProvider : public UObject
{
	GENERATED_BODY()
	
public:
	UTrajectoryDataProvider();

	UFUNCTION(BlueprintCallable, Category = "Trajectory Data")
	static UDataTable* GetDefaultTrajectoryTableByTag(FGameplayTag SourceTag) { return GetDefault<UTrajectoryDataProvider>()->GetTrajectoryTableByTag(SourceTag); }

	UDataTable* GetTrajectoryTableByTag(FGameplayTag SourceTag) const;

protected:
	UPROPERTY(EditDefaultsOnly)
	UDataTable* SourceTrajectoryMappingDT;
};
