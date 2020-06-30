// Fill out your copyright notice in the Description page of Project Settings.


#include "TennisStoryGameInstance.h"
#include "AbilitySystemGlobals.h"

UTennisStoryGameInstance::UTennisStoryGameInstance()
{
	CurrentOnlinePlayType = EOnlinePlayType::Offline;
}

void UTennisStoryGameInstance::Init()
{
	Super::Init();

	//Need to call this in order to use FGameplayAbilityTargetData for the AbilitySystem, currently in use for the DiveAbility
	UAbilitySystemGlobals::Get().InitGlobalData();
}
