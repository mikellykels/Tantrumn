// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "EquippedNameWidget.h"
#include "GameFramework/GameModeBase.h"
#include "GetNameInterface.h"
#include "TantrumnGameWidget.h"
#include "TantrumnPausedWidget.h"
#include "ThrowableActor.h"
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

	void DisplayEquippedWidget();
	void DisplayEquippedName(AThrowableActor* InThrowableActor, APlayerController* PlayerController);
	void DisplayBuffName(AThrowableActor* InThrowableActor, APlayerController* PlayerController);

	void RemoveEquippedWidget(APlayerController* PlayerController);
	void RemoveEquippedName(APlayerController* PlayerController);
	void RemoveBuffName(APlayerController* PlayerController);

	IGetNameInterface* GetNameInterface;

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

	UPROPERTY()
	AThrowableActor* ThrowableActor;

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

	UPROPERTY()
	TMap <APlayerController*, UEquippedNameWidget*> EquippedNameWidgets;

	UPROPERTY(EditAnywhere, Category = "HUD")
	TSubclassOf<UEquippedNameWidget> EquippedNameWidgetClass;

	// --- FUNCTIONS --- //

	void AttemptStartGame();
	void DisplayCountdown();
	void StartGame();
	void DisplayPausedMenu();
};
