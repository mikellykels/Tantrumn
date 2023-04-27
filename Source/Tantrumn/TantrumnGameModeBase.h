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
	virtual void RestartPlayer(AController* NewPlayer) override;

	UFUNCTION(BlueprintCallable)
	EGameState GetCurrentGameState() const;

	void PlayerReachedEnd(APlayerController* PlayerController);
	void ReceivePlayer(APlayerController* PlayerController);
	void PlayerPausedGame(APlayerController* PlayerController);

	UFUNCTION(BlueprintCallable)
	void PlayerResumedGame();

private:

	// --- VARS --- //

	// Create and set CurrentGameState to NONE. This will be tracked in the code file.
	UPROPERTY(VisibleAnywhere, Category = "States")
	EGameState CurrentGameState = EGameState::NONE;
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

	// object we'll be creating and adding to the viewport
	UPROPERTY()
	TMap<APlayerController*, UTantrumnGameWidget*> GameWidgets;

	UPROPERTY(EditAnywhere, Category = "Widget")
	TSubclassOf<UTantrumnGameWidget> GameWidgetClass;

	UPROPERTY()
	TMap<APlayerController*, UTantrumnPausedWidget*> PausedWidgets;

	UPROPERTY(EditAnywhere, Category = "Widget")
	TSubclassOf<UTantrumnPausedWidget> PausedWidgetClass;

	// --- FUNCTIONS --- //

	void AttemptStartGame();
	void DisplayCountdown();
	void StartGame();
	void DisplayPausedMenu();
};
