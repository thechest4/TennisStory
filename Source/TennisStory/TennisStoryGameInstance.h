// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "TennisStoryGameInstance.generated.h"

UENUM(BlueprintType)
enum class EOnlinePlayType : uint8
{
	Offline,
	Online
};

UCLASS()
class TENNISSTORY_API UTennisStoryGameInstance : public UGameInstance
{
	GENERATED_BODY()
	
public:
	UTennisStoryGameInstance();

	EOnlinePlayType GetOnlinePlayType()
	{
		return CurrentOnlinePlayType;
	}

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Game Instance")
	EOnlinePlayType CurrentOnlinePlayType;
};
