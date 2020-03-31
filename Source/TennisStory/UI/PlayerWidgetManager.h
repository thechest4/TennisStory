// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PlayerWidgetManager.generated.h"

class UGameMenuWidget;
class UReadyUpWidget;
class UPlayerReadyStatusWidget;

UENUM()
enum class EInputMode : uint8
{
	GameMode,
	UIMode
};

UCLASS()
class TENNISSTORY_API UPlayerWidgetManager : public UUserWidget
{
	GENERATED_BODY()
	
public:
	UPlayerWidgetManager(const FObjectInitializer& ObjectInitializer);

	UFUNCTION(BlueprintCallable, Category = "Player Widget Manager")
	UGameMenuWidget* GetGameMenuWidgetObject() { return GameMenuWidgetObject; }
	
	UFUNCTION(BlueprintCallable, Category = "Player Widget Manager")
	UReadyUpWidget* GetReadyUpWidgetObject() { return ReadyUpWidgetObject; }
	
	UFUNCTION(BlueprintCallable, Category = "Player Widget Manager")
	UPlayerReadyStatusWidget* GetReadyStateWidgetObject() { return ReadyStateWidgetObject; }

	UFUNCTION(BlueprintCallable, Category = "Player Widget Manager")
	void ShowGameMenu();

	UFUNCTION(BlueprintCallable, Category = "Player Widget Manager")
	void HideGameMenu();

	UFUNCTION(BlueprintCallable, Category = "Player Widget Manager")
	void ShowReadyWidgets();

	UFUNCTION(BlueprintCallable, Category = "Player Widget Manager")
	void HideReadyWidgets();

	UFUNCTION(BlueprintCallable, Category = "Player Widget Manager")
	void HideAllWidgets();

protected:
	virtual void NativeOnInitialized() override;

	//Sets the c++ pointers to their instances implemented in blueprint
	UFUNCTION(BlueprintImplementableEvent, Category = "Player Widget Manager")
	void CatalogWidgetObjects();

	UPROPERTY(BlueprintReadWrite, Category = "Player Widget Manager")
	UGameMenuWidget* GameMenuWidgetObject;
	
	UPROPERTY(BlueprintReadWrite, Category = "Player Widget Manager")
	UReadyUpWidget* ReadyUpWidgetObject;
	
	UPROPERTY(BlueprintReadWrite, Category = "Player Widget Manager")
	UPlayerReadyStatusWidget* ReadyStateWidgetObject;

	UPROPERTY()
	TArray<UUserWidget*> AllWidgets;

	UPROPERTY()
	TArray<UUserWidget*> WidgetsWantingUIInput;

	EInputMode CurrentInputMode;

	void SetAppropriateInputMode();

	void SetPlayerInputToUIMode();
	
	void SetPlayerInputToGameMode();
};
