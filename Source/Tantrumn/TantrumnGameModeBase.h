// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "TantrumnGameModeBase.generated.h"

class AController;
class ATantrumnGameStateBase;
class ATantrumnPlayerController;
class AThrowableActor;
class UTantrumnGameWidget;
class UTantrumnPausedWidget;
class UEquippedNameWidget;
class IGetNameInterface;

UCLASS()
class TANTRUMN_API ATantrumnGameModeBase : public AGameModeBase
{
	GENERATED_BODY()
	
public:

	// --- FUNCTIONS --- //

	ATantrumnGameModeBase();

	virtual void BeginPlay() override;
	virtual void RestartPlayer(AController* NewPlayer) override;
	void DisplayCountdown(float GameCountdownDuration, ATantrumnPlayerController* TantrumnPlayerController);
	void DisplayLevelComplete(ATantrumnPlayerController* TantrumnPlayerController);

	void RestartGame();
	void PauseGame(APlayerController* PlayerController);
	
	UFUNCTION(BlueprintCallable)
	void ResumeGame(APlayerController* PlayerController);

	void DisplayEquippedWidget(APlayerController* PlayerController);
	void DisplayEquippedName(AThrowableActor* InThrowableActor, APlayerController* PlayerController);
	void DisplayBuffName(AThrowableActor* InThrowableActor, APlayerController* PlayerController);

	void RemoveEquippedWidget(APlayerController* PlayerController);
	void RemoveEquippedName(APlayerController* PlayerController);
	void RemoveBuffName(APlayerController* PlayerController);

	IGetNameInterface* GetNameInterface;

private:

	// --- VARS --- //

	UPROPERTY()
	TMap<APlayerController*, UTantrumnGameWidget*> GameWidgets;

	UPROPERTY(EditAnywhere, Category = "Widget")
	TSubclassOf<UTantrumnGameWidget> GameWidgetClass;

	UPROPERTY()
	TMap<APlayerController*, UTantrumnPausedWidget*> PausedWidgets;

	UPROPERTY(EditAnywhere, Category = "Widget")
	TSubclassOf<UTantrumnPausedWidget> PausedWidgetClass;

	UPROPERTY()
	TMap <APlayerController*, UEquippedNameWidget*> EquippedNameWidgets;

	UPROPERTY(EditAnywhere, Category = "HUD")
	TSubclassOf<UEquippedNameWidget> EquippedNameWidgetClass;

	// Countdown before gameplay state begins. Exposed so we can easily change this in BP editor.
	UPROPERTY(EditAnywhere, Category = "Game Details")
	float GameCountdownDuration = 3.0f;

	UPROPERTY(EditAnywhere, Category = "Game Details")
	uint8 NumExpectedPlayers = 1u;

	UFUNCTION(BlueprintCallable, Category = "Game Details")
	void SetNumExpectedPlayers(uint8 InNumExpectedPlayers)
	{
		NumExpectedPlayers = InNumExpectedPlayers;
	}

	FTimerHandle TimerHandle;

	UPROPERTY()
	AThrowableActor* ThrowableActor;

	UPROPERTY()
	ATantrumnGameStateBase* TantrumnGameStateBase;

	// --- FUNCTIONS --- //

	void AttemptStartGame();
	void StartGame();
	void DisplayPausedMenu();

protected:
	UFUNCTION()
	void OnGameStateSet(AGameStateBase* GameStateBase);
};
