// Copyright Epic Games, Inc. All Rights Reserved.


#include "TantrumnGameModeBase.h"
#include "TantrumnGameWidget.h"
#include "TantrumnGameInstance.h"
#include "TantrumnGameStateBase.h"
#include "TantrumnEnemyAIController.h"
#include "TantrumnPausedWidget.h"
#include "TantrumnPlayerController.h"
#include "TantrumnPlayerState.h"
#include "ThrowableActor.h"
#include "EquippedNameWidget.h"
#include "GetNameInterface.h"
#include "Components/TextBlock.h"
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
			UTantrumnPausedWidget** PausedWidget = PausedWidgets.Find(PlayerController);
			if (PausedWidget)
			{
				UGameplayStatics::SetGamePaused(GetWorld(), false);
				(*PausedWidget)->RemoveFromParent();
				//restore game input 
				PlayerController->SetInputMode(FInputModeGameOnly());
				PlayerController->SetShowMouseCursor(false);
			}
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

void ATantrumnGameModeBase::DisplayLevelComplete(ATantrumnPlayerController* TantrumnPlayerController)
{
	UTantrumnGameWidget** GameWidget = GameWidgets.Find(TantrumnPlayerController);
	if (GameWidget)
	{
		(*GameWidget)->DisplayResults();
	}
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
			if (ATantrumnPlayerController* TantrumnPlayerController = Cast<ATantrumnPlayerController>(PlayerController))
			{
				TantrumnPlayerController->ClientRestartGame();
			}
			RestartPlayer(PlayerController);
		}
	}
}

void ATantrumnGameModeBase::PauseGame(APlayerController* PlayerController)
{
	UTantrumnGameWidget** GameWidget = GameWidgets.Find(PlayerController);

	DisplayPausedMenu();
	UGameplayStatics::SetGamePaused(GetWorld(), true);

	PlayerController->SetInputMode(FInputModeUIOnly());
	PlayerController->SetShowMouseCursor(true);
}

void ATantrumnGameModeBase::ResumeGame(APlayerController* InPlayerController)
{
		for (FConstPlayerControllerIterator Iterator = GetWorld()->GetPlayerControllerIterator(); Iterator; ++Iterator)
		{
			APlayerController* PlayerController = Iterator->Get();
			if (PlayerController && PlayerController->PlayerState && !MustSpectate(PlayerController))
			{
				UTantrumnPausedWidget** PausedWidget = PausedWidgets.Find(PlayerController);
				UGameplayStatics::SetGamePaused(GetWorld(), false);
				(*PausedWidget)->RemoveFromParent();

				PlayerController->SetInputMode(FInputModeGameOnly());
				PlayerController->SetShowMouseCursor(false);
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
		for (FConstPlayerControllerIterator Iterator = GetWorld()->GetPlayerControllerIterator(); Iterator; ++Iterator)
		{
			APlayerController* PlayerController = Iterator->Get();
			if (PlayerController && PlayerController->PlayerState && !MustSpectate(PlayerController))
			{
				if (ATantrumnPlayerController* TantrumnPlayerController = Cast<ATantrumnPlayerController>(PlayerController))
				{
					TantrumnPlayerController->ClientDisplayCountdown(GameCountdownDuration, GameWidgetClass);
				}
			}
		}

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

void ATantrumnGameModeBase::DisplayCountdown(float InGameCountdownDuration, ATantrumnPlayerController* TantrumnPlayerController)
{
	if (GetWorld())
	{
		UTantrumnGameWidget* PlayerGameWidget = nullptr;
		UTantrumnGameWidget** GameWidget = GameWidgets.Find(TantrumnPlayerController);
		if (!GameWidget)
		{
			PlayerGameWidget = CreateWidget<UTantrumnGameWidget>(TantrumnPlayerController, GameWidgetClass);
			if (PlayerGameWidget)
			{
				//only do this once
				//atm we never remove this as it has the race time
				PlayerGameWidget->AddToPlayerScreen();
				GameWidgets.Add(TantrumnPlayerController, PlayerGameWidget);
			}
		}
		else
		{
			PlayerGameWidget = *GameWidget;
		}

		if (PlayerGameWidget)
		{
			PlayerGameWidget->StartCountdown(InGameCountdownDuration, TantrumnPlayerController);
			TantrumnGameStateBase = GetWorld()->GetGameState<ATantrumnGameStateBase>();
			if (!TantrumnGameStateBase)
			{
				GetWorld()->GameStateSetEvent.AddUObject(this, &ATantrumnGameModeBase::OnGameStateSet);
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
			DisplayEquippedWidget(PlayerController);
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
				PausedWidget->LevelPaused(PlayerController);
				PausedWidget->AddToPlayerScreen();
				PausedWidgets.Add(PlayerController, PausedWidget);
			}
		}
	}
}

void ATantrumnGameModeBase::OnGameStateSet(AGameStateBase* GameStateBase)
{
	TantrumnGameStateBase = Cast<ATantrumnGameStateBase>(GameStateBase);
}

void ATantrumnGameModeBase::DisplayEquippedWidget(APlayerController* PlayerController)
{
	if (!EquippedNameWidgetClass)
	{
		return;
	}

	if (UEquippedNameWidget* EquippedNameWidget = CreateWidget<UEquippedNameWidget>(PlayerController, EquippedNameWidgetClass))
	{
		EquippedNameWidget->AddToPlayerScreen();
		EquippedNameWidgets.Add(PlayerController, EquippedNameWidget);
	}
}

void ATantrumnGameModeBase::DisplayEquippedName(AThrowableActor* InThrowableActor, APlayerController* PlayerController)
{
	ThrowableActor = InThrowableActor;
	UEquippedNameWidget** EquippedNameWidget = EquippedNameWidgets.Find(PlayerController);

	if (EquippedNameWidget)
	{
		GetNameInterface = Cast<IGetNameInterface>(ThrowableActor);
		if (GetNameInterface)
		{
			if (ThrowableActor->GetClass()->ImplementsInterface(UGetNameInterface::StaticClass()))
			{
				FText Name = GetNameInterface->GetName();
				(*EquippedNameWidget)->EquippedName->SetText(Name);
			}
		}
	}
}

void ATantrumnGameModeBase::DisplayBuffName(AThrowableActor* InThrowableActor, APlayerController* PlayerController)
{
	ThrowableActor = InThrowableActor;
	UEquippedNameWidget** EquippedNameWidget = EquippedNameWidgets.Find(PlayerController);

	if (EquippedNameWidget)
	{
		GetNameInterface = Cast<IGetNameInterface>(ThrowableActor);
		if (GetNameInterface)
		{
			if (ThrowableActor->GetClass()->ImplementsInterface(UGetNameInterface::StaticClass()))
			{
				FText Name = GetNameInterface->GetName();
				(*EquippedNameWidget)->BuffName->SetText(Name);
			}
		}
	}
}

void ATantrumnGameModeBase::RemoveEquippedWidget(APlayerController* PlayerController)
{
	UEquippedNameWidget** EquippedNameWidget = EquippedNameWidgets.Find(PlayerController);

	if (EquippedNameWidget)
	{
		(*EquippedNameWidget)->RemoveFromParent();
	}
}

void ATantrumnGameModeBase::RemoveEquippedName(APlayerController* PlayerController)
{
	UEquippedNameWidget** EquippedNameWidget = EquippedNameWidgets.Find(PlayerController);

	if (EquippedNameWidget)
	{
		(*EquippedNameWidget)->EquippedName->SetText(FText::FromString(""));
	}
}

void ATantrumnGameModeBase::RemoveBuffName(APlayerController* PlayerController)
{
	UEquippedNameWidget** EquippedNameWidget = EquippedNameWidgets.Find(PlayerController);

	if (EquippedNameWidget)
	{
		(*EquippedNameWidget)->BuffName->SetText(FText::FromString(""));
	}
}