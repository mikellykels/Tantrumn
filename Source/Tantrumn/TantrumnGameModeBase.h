// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "TantrumnGameModeBase.generated.h"

class AController;
class ATantrumnPlayerController;

UCLASS()
class TANTRUMN_API ATantrumnGameModeBase : public AGameModeBase
{
	GENERATED_BODY()
	
public:

	// --- FUNCTIONS --- //

	ATantrumnGameModeBase();

	virtual void BeginPlay() override;
	virtual void RestartPlayer(AController* NewPlayer) override;

	void RestartGame();
	void PauseGame();
	void DisplayEquippedWidget();

private:

	// --- VARS --- //

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

	// --- FUNCTIONS --- //

	void AttemptStartGame();
	void DisplayCountdown();
	void StartGame();
};
