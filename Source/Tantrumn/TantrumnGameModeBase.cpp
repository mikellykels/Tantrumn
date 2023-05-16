// Copyright Epic Games, Inc. All Rights Reserved.


#include "TantrumnGameModeBase.h"
#include "TantrumnGameWidget.h"
#include "TantrumnGameInstance.h"
#include "TantrumnGameStateBase.h"
#include "TantrumnEnemyAIController.h"
#include "TantrumnPlayerController.h"
#include "TantrumnPlayerState.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"

ATantrumnGameModeBase::ATantrumnGameModeBase()
{
	PrimaryActorTick.bCanEverTick = false;
}

void ATantrumnGameModeBase::BeginPlay()
{
	Super::BeginPlay();

	if (ATantrumnGameStateBase* TantrumnGameState = GetGameState<ATantrumnGameStateBase>())
	{
		TantrumnGameState->SetGameState(EGameState::Waiting);
	}
}

void ATantrumnGameModeBase::RestartPlayer(AController* NewPlayer)
{
	Super::RestartPlayer(NewPlayer);

	if (APlayerController* PlayerController = Cast<APlayerController>(NewPlayer))
	{
		if (PlayerController->GetCharacter() && PlayerController->GetCharacter()->GetCharacterMovement())
		{
			PlayerController->GetCharacter()->GetCharacterMovement()->SetMovementMode(MOVE_Walking);
			ATantrumnPlayerState* PlayerState = PlayerController->GetPlayerState<ATantrumnPlayerState>();
			if (PlayerState)
			{
				PlayerState->SetCurrentState(EPlayerGameState::Waiting);
			}
		}
	}
	AttemptStartGame();
}

void ATantrumnGameModeBase::RestartGame()
{
	// destroy actor
	for (FConstControllerIterator Iterator = GetWorld()->GetControllerIterator(); Iterator; ++Iterator)
	{
		ATantrumnEnemyAIController* TantrumnEnemyAIController = Cast<ATantrumnEnemyAIController>(Iterator->Get());
		if (TantrumnEnemyAIController && TantrumnEnemyAIController->GetPawn())
		{
			TantrumnEnemyAIController->Destroy(true);
		}
	}

	ResetLevel();
	//RestartGame();
	//GetWorld()->ServerTravel(TEXT("?Restart"), false);
	//ProcessServerTravel("?Restart");
	for (FConstPlayerControllerIterator Iterator = GetWorld()->GetPlayerControllerIterator(); Iterator; ++Iterator)
	{
		APlayerController* PlayerController = Iterator->Get();
		if (PlayerController && PlayerController->PlayerState && !MustSpectate(PlayerController))
		{
			//call something to clean up the hud 
			if (ATantrumnPlayerController* TantrumnPlayerController = Cast< ATantrumnPlayerController>(PlayerController))
			{
				TantrumnPlayerController->ClientRestartGame();
			}
			RestartPlayer(PlayerController);
		}
	}
}

void ATantrumnGameModeBase::PauseGame()
{
	for (FConstPlayerControllerIterator Iterator = GetWorld()->GetPlayerControllerIterator(); Iterator; ++Iterator)
	{
		APlayerController* PlayerController = Iterator->Get();
		if (PlayerController && PlayerController->PlayerState && !MustSpectate(PlayerController))
		{
			//call something to clean up the hud 
			if (ATantrumnPlayerController* TantrumnPlayerController = Cast< ATantrumnPlayerController>(PlayerController))
			{
				TantrumnPlayerController->ClientPauseGame();
			}
		}
	}
}

void ATantrumnGameModeBase::AttemptStartGame()
{
	if (ATantrumnGameStateBase* TantrumnGameState = GetGameState<ATantrumnGameStateBase>())
	{
		TantrumnGameState->SetGameState(EGameState::Waiting);
	}

	if (GetNumPlayers() == NumExpectedPlayers)
	{
		DisplayCountdown();
		DisplayEquippedWidget();
		if (GameCountdownDuration > SMALL_NUMBER)
		{
			GetWorld()->GetTimerManager().SetTimer(TimerHandle, this, &ATantrumnGameModeBase::StartGame, GameCountdownDuration, false);
		}
		else
		{
			StartGame();
		}
	}
}

void ATantrumnGameModeBase::DisplayCountdown()
{
	for (FConstPlayerControllerIterator Iterator = GetWorld()->GetPlayerControllerIterator(); Iterator; ++Iterator)
	{
		APlayerController* PlayerController = Iterator->Get();
		if (PlayerController && PlayerController->PlayerState && !MustSpectate(PlayerController))
		{
			if (ATantrumnPlayerController* TantrumnPlayerController = Cast< ATantrumnPlayerController>(PlayerController))
			{
				TantrumnPlayerController->ClientDisplayCountdown(GameCountdownDuration);
			}
		}
	}
}

void ATantrumnGameModeBase::StartGame()
{
	if (ATantrumnGameStateBase* TantrumnGameState = GetGameState<ATantrumnGameStateBase>())
	{
		TantrumnGameState->SetGameState(EGameState::Playing);
		TantrumnGameState->ClearResults();
	}

	for (FConstControllerIterator Iterator = GetWorld()->GetControllerIterator(); Iterator; ++Iterator)
	{
		ATantrumnEnemyAIController* TantrumnEnemyAIController = Cast<ATantrumnEnemyAIController>(Iterator->Get());
		if (TantrumnEnemyAIController && TantrumnEnemyAIController->GetPawn())
		{
			ATantrumnPlayerState* PlayerState = TantrumnEnemyAIController->GetPlayerState<ATantrumnPlayerState>();
			if (PlayerState)
			{
				PlayerState->SetCurrentState(EPlayerGameState::Playing);
				PlayerState->SetIsWinner(false);
			}
		}
	}

	for (FConstPlayerControllerIterator Iterator = GetWorld()->GetPlayerControllerIterator(); Iterator; ++Iterator)
	{
		APlayerController* PlayerController = Iterator->Get();
		if (PlayerController && PlayerController->PlayerState && !MustSpectate(PlayerController))
		{
			PlayerController->SetInputMode(FInputModeGameOnly());
			PlayerController->SetShowMouseCursor(false);

			ATantrumnPlayerState* PlayerState = PlayerController->GetPlayerState<ATantrumnPlayerState>();
			if (PlayerState)
			{
				PlayerState->SetCurrentState(EPlayerGameState::Playing);
				PlayerState->SetIsWinner(false);
			}
		}
	}
}

void ATantrumnGameModeBase::DisplayEquippedWidget()
{
	for (FConstPlayerControllerIterator Iterator = GetWorld()->GetPlayerControllerIterator(); Iterator; ++Iterator)
	{
		APlayerController* PlayerController = Iterator->Get();
		if (PlayerController && PlayerController->PlayerState && !MustSpectate(PlayerController))
		{
			if (ATantrumnPlayerController* TantrumnPlayerController = Cast< ATantrumnPlayerController>(PlayerController))
			{
				TantrumnPlayerController->ClientDisplayEquippedWidget();
			}
		}
	}
}