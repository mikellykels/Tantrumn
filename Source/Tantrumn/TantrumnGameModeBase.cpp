// Copyright Epic Games, Inc. All Rights Reserved.


#include "TantrumnGameModeBase.h"
#include "TantrumnGameWidget.h"
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

	CurrentGameState = EGameState::Waiting;
	//DisplayCountdown();
	//GetWorld()->GetTimerManager().SetTimer(TimerHandle, this, &ATantrumnGameModeBase::StartGame, GameCountdownDuration, false);
}

void ATantrumnGameModeBase::RestartPlayer(AController* NewPlayer)
{
	Super::RestartPlayer(NewPlayer);

	if (APlayerController* PlayerController = Cast<APlayerController>(NewPlayer))
	{
		if (PlayerController->GetCharacter() && PlayerController->GetCharacter()->GetCharacterMovement())
		{
			PlayerController->GetCharacter()->GetCharacterMovement()->SetMovementMode(MOVE_Walking);
		}
	}
}

EGameState ATantrumnGameModeBase::GetCurrentGameState() const
{
	return CurrentGameState;
}

void ATantrumnGameModeBase::PlayerReachedEnd(APlayerController* PlayerController)
{
	CurrentGameState = EGameState::GameOver;
	UTantrumnGameWidget** GameWidget = GameWidgets.Find(PlayerController);

	if (GameWidget)
	{
		(*GameWidget)->LevelComplete();
		PlayerController->SetInputMode(FInputModeUIOnly());
		PlayerController->SetShowMouseCursor(true);

		if (PlayerController->GetCharacter() && PlayerController->GetCharacter()->GetCharacterMovement())
		{
			PlayerController->GetCharacter()->GetCharacterMovement()->DisableMovement();
		}
	}
}

void ATantrumnGameModeBase::ReceivePlayer(APlayerController* PlayerController)
{
	AttemptStartGame();
}

void ATantrumnGameModeBase::PlayerPausedGame(APlayerController* PlayerController)
{
	CurrentGameState = EGameState::Paused;

	UTantrumnGameWidget** GameWidget = GameWidgets.Find(PlayerController);

	DisplayPausedMenu();

	if (GameWidget)
	{
		UGameplayStatics::SetGamePaused(GetWorld(), true);

		PlayerController->SetInputMode(FInputModeUIOnly());
		PlayerController->SetShowMouseCursor(true);

		(*GameWidget)->SetVisibility(ESlateVisibility::Hidden);
	}
}

void ATantrumnGameModeBase::PlayerResumedGame()
{
	CurrentGameState = EGameState::Playing;
	UGameplayStatics::SetGamePaused(GetWorld(), false);

	for (FConstPlayerControllerIterator Iterator = GetWorld()->GetPlayerControllerIterator(); Iterator; ++Iterator)
	{
		APlayerController* PlayerController = Iterator->Get();
		if (PlayerController && PlayerController->PlayerState && !MustSpectate(PlayerController))
		{
			UTantrumnGameWidget** GameWidget = GameWidgets.Find(PlayerController);
			UTantrumnPausedWidget** PausedWidget = PausedWidgets.Find(PlayerController);

			if (GameWidget && PausedWidget)
			{
				(*PausedWidget)->RemoveFromParent();
				PlayerController->SetInputMode(FInputModeGameOnly());
				PlayerController->SetShowMouseCursor(false);

				(*GameWidget)->SetVisibility(ESlateVisibility::Visible);
			}
		}
	}
}

void ATantrumnGameModeBase::AttemptStartGame()
{
	if (GetNumPlayers() == NumExpectedPlayers)
	{
		DisplayCountdown();
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
	if (!GameWidgetClass)
	{
		GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, TEXT("!GameWidgetClass"));
		return;
	}

	for (FConstPlayerControllerIterator Iterator = GetWorld()->GetPlayerControllerIterator(); Iterator; ++Iterator)
	{
		APlayerController* PlayerController = Iterator->Get();
		if (PlayerController && PlayerController->PlayerState && !MustSpectate(PlayerController))
		{
			if (UTantrumnGameWidget* GameWidget = CreateWidget<UTantrumnGameWidget>(PlayerController, GameWidgetClass))
			{
				GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, TEXT("Add"));
				GameWidget->AddToPlayerScreen();
				GameWidget->StartCountdown(GameCountdownDuration, this);
				GameWidgets.Add(PlayerController, GameWidget);
			}
		}
	}
}

void ATantrumnGameModeBase::StartGame()
{
	CurrentGameState = EGameState::Playing;

	for (FConstPlayerControllerIterator Iterator = GetWorld()->GetPlayerControllerIterator(); Iterator; ++Iterator)
	{
		APlayerController* PlayerController = Iterator->Get();
		if (PlayerController && PlayerController->PlayerState && !MustSpectate(PlayerController))
		{
			PlayerController->SetInputMode(FInputModeGameOnly());
			PlayerController->SetShowMouseCursor(false);
		}
	}
}

void ATantrumnGameModeBase::DisplayPausedMenu()
{
	if (!PausedWidgetClass)
	{
		return;
	}

	for (FConstPlayerControllerIterator Iterator = GetWorld()->GetPlayerControllerIterator(); Iterator; ++Iterator)
	{
		APlayerController* PlayerController = Iterator->Get();
		if (PlayerController && PlayerController->PlayerState && !MustSpectate(PlayerController))
		{
			if (UTantrumnPausedWidget* PausedWidget = CreateWidget<UTantrumnPausedWidget>(PlayerController, PausedWidgetClass))
			{
				PausedWidget->LevelPaused(this);
				PausedWidget->AddToPlayerScreen();
				PausedWidgets.Add(PlayerController, PausedWidget);
			}
		}
	}
}
