// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "GetNameInterface.h"
#include "TantrumnGameInstance.generated.h"

class ATantrumnPlayerController;
class ATantrumnGameStateBase;
class AThrowableActor;
class UTantrumnGameWidget;
class UTantrumnPausedWidget;
class UEquippedNameWidget;

UCLASS()
class TANTRUMN_API UTantrumnGameInstance : public UGameInstance
{
	GENERATED_BODY()
	
public:
	void DisplayCountdown(float GameCountdownDuration, ATantrumnPlayerController* TantrumnPlayerController);
	void DisplayLevelComplete(ATantrumnPlayerController* TantrumnPlayerController);
	void DisplayPausedMenu(ATantrumnPlayerController* TantrumnPlayerController);
	void DisplayEquippedWidget(ATantrumnPlayerController* TantrumnPlayerController);
	void DisplayEquippedName(AThrowableActor* InThrowableActor, APlayerController* PlayerController);
	void DisplayBuffName(AThrowableActor* InThrowableActor, APlayerController* PlayerController);

	//void RemoveEquippedWidget(APlayerController* PlayerController);
	void RemoveEquippedName(APlayerController* PlayerController);
	void RemoveBuffName(APlayerController* PlayerController);

	UFUNCTION(BlueprintPure)
	ATantrumnGameStateBase* GetGameState() const { 
		return TantrumnGameStateBase; 
	}

	UFUNCTION(BlueprintCallable)
	void OnRetrySelected(ATantrumnPlayerController* TantrumnPlayerController);

	UFUNCTION()
	void RestartGame(ATantrumnPlayerController* TantrumnPlayerController);

	UFUNCTION()
	void PauseGame(ATantrumnPlayerController* TantrumnPlayerController);

	UFUNCTION(BlueprintCallable)
	void ResumeGame(ATantrumnPlayerController* TantrumnPlayerController);

	IGetNameInterface* GetNameInterface;

protected:
	UFUNCTION()
	void OnGameStateSet(AGameStateBase* GameStateBase);

private:
	UPROPERTY(EditAnywhere, Category = "Widget")
	TSubclassOf<UTantrumnGameWidget> GameWidgetClass; // Exposed class to check the type of widget to display

	UPROPERTY()
	TMap<APlayerController*, UTantrumnGameWidget*> GameWidgets;

	UPROPERTY()
	TMap<APlayerController*, UTantrumnPausedWidget*> PausedWidgets;

	UPROPERTY(EditAnywhere, Category = "Widget")
	TSubclassOf<UTantrumnPausedWidget> PausedWidgetClass;

	UPROPERTY()
	TMap <APlayerController*, UEquippedNameWidget*> EquippedNameWidgets;

	UPROPERTY(EditAnywhere, Category = "HUD")
	TSubclassOf<UEquippedNameWidget> EquippedNameWidgetClass;

	UPROPERTY()
	ATantrumnGameStateBase* TantrumnGameStateBase;

	AThrowableActor* ThrowableActor;
};