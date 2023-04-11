// Copyright Epic Games, Inc. All Rights Reserved.


#include "TantrumnGameModeBase.h"
#include "Kismet/GameplayStatics.h"

ATantrumnGameModeBase::ATantrumnGameModeBase()
{

}

void ATantrumnGameModeBase::BeginPlay()
{
	Super::BeginPlay();

	CurrentGameState = EGameState::Waiting;
	DisplayCountdown();
	GetWorld()->GetTimerManager().SetTimer(TimerHandle, this, &ATantrumnGameModeBase::StartGame, GameCountdownDuration, false);
}

EGameState ATantrumnGameModeBase::GetCurrentGameState() const
{
	return CurrentGameState;
}

void ATantrumnGameModeBase::PlayerReachedEnd()
{
	CurrentGameState = EGameState::GameOver;
	
	GameWidget->LevelComplete();

	PC->SetInputMode(FInputModeUIOnly());
	PC->SetShowMouseCursor(true);
}

void ATantrumnGameModeBase::PlayerPausedGame()
{
	CurrentGameState = EGameState::Paused;

	DisplayPausedMenu();
	PausedWidget->LevelPaused(this);

	UGameplayStatics::SetGamePaused(GetWorld(), true);
	PC->SetInputMode(FInputModeUIOnly());
	PC->SetShowMouseCursor(true);

	GameWidget->SetVisibility(ESlateVisibility::Hidden);
}

void ATantrumnGameModeBase::PlayerResumedGame()
{
	CurrentGameState = EGameState::Playing;
	PausedWidget->RemoveFromParent();
	GameWidget->SetVisibility(ESlateVisibility::Visible);

	UGameplayStatics::SetGamePaused(GetWorld(), false);
	PC->SetInputMode(FInputModeGameOnly());
	PC->SetShowMouseCursor(false);
}

void ATantrumnGameModeBase::DisplayCountdown()
{
	if (!GameWidgetClass)
	{
		return;
	}

	PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
	GameWidget = CreateWidget<UTantrumnGameWidget>(PC, GameWidgetClass);
	GameWidget->AddToViewport();
	GameWidget->StartCountdown(GameCountdownDuration, this);
}

void ATantrumnGameModeBase::StartGame()
{
	CurrentGameState = EGameState::Playing;

	PC->SetInputMode(FInputModeGameOnly());
	PC->SetShowMouseCursor(false);
}

void ATantrumnGameModeBase::DisplayPausedMenu()
{
	if (!PausedWidgetClass)
	{
		return;
	}

	PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
	PausedWidget = CreateWidget<UTantrumnPausedWidget>(PC, PausedWidgetClass);
	PausedWidget->AddToViewport();
}
