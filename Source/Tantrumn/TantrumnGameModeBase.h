// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "TantrumnGameWidget.h"
#include "TantrumnPausedWidget.h"
#include "TantrumnGameModeBase.generated.h"

/**
 * 
 */

// Enum to track the current state of the game
UENUM(BlueprintType)
enum class EGameState : uint8
{
	NONE          UMETA(DisplayName = "NONE"),
	Waiting       UMETA(DisplayName = "Waiting"),
	Playing       UMETA(DisplayName = "Playing"),
	Paused        UMETA(DisplayName = "Paused"),
	GameOver      UMETA(DisplayName = "GameOver"),
};
UCLASS()
class TANTRUMN_API ATantrumnGameModeBase : public AGameModeBase
{
	GENERATED_BODY()
	
public:

	// --- FUNCTIONS --- //

	ATantrumnGameModeBase();

	virtual void BeginPlay() override;

	UFUNCTION(BlueprintCallable)
	EGameState GetCurrentGameState() const;

	void PlayerReachedEnd();
	void PlayerPausedGame();

	UFUNCTION(BlueprintCallable)
	void PlayerResumedGame();

private:

	// --- VARS --- //

	// Create and set CurrentGameState to NONE. This will be tracked in the code file.
	UPROPERTY(VisibleAnywhere, Category = "States")
	EGameState CurrentGameState = EGameState::NONE;
	// Countdown before gameplay state begins. Exposed so we can easily change this in BP editor.
	UPROPERTY(EditAnywhere, Category = "GameDetails")
	float GameCountdownDuration = 3.0f;

	FTimerHandle TimerHandle;

	UPROPERTY()
	UTantrumnGameWidget* GameWidget;

	UPROPERTY(EditAnywhere, Category = "Widget")
	TSubclassOf<UTantrumnGameWidget> GameWidgetClass;

	UPROPERTY()
	UTantrumnPausedWidget* PausedWidget;

	UPROPERTY(EditAnywhere, Category = "Widget")
	TSubclassOf<UTantrumnPausedWidget> PausedWidgetClass;

	APlayerController* PC = nullptr;

	// --- FUNCTIONS --- //

	void DisplayCountdown();
	void StartGame();
	void DisplayPausedMenu();
};
