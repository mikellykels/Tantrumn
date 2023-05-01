// Copyright Epic Games, Inc. All Rights Reserved.


#include "TantrumnGameModeBase.h"
#include "TantrumnGameWidget.h"
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

	CurrentGameState = EGameState::Waiting;
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
		(*GameWidget)->SetKeyboardFocus();

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
	if (!GameWidgetClass)
	{
		return;
	}

	for (FConstPlayerControllerIterator Iterator = GetWorld()->GetPlayerControllerIterator(); Iterator; ++Iterator)
	{
		APlayerController* PlayerController = Iterator->Get();
		if (PlayerController && PlayerController->PlayerState && !MustSpectate(PlayerController))
		{
			if (UTantrumnGameWidget* GameWidget = CreateWidget<UTantrumnGameWidget>(PlayerController, GameWidgetClass))
			{
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

void ATantrumnGameModeBase::DisplayEquippedWidget()
{
	if (!EquippedNameWidgetClass)
	{
		return;
	}
	for (FConstPlayerControllerIterator Iterator = GetWorld()->GetPlayerControllerIterator(); Iterator; ++Iterator)
	{
		APlayerController* PlayerController = Iterator->Get();
		if (PlayerController && PlayerController->PlayerState && !MustSpectate(PlayerController))
		{
			if (UEquippedNameWidget* EquippedNameWidget = CreateWidget<UEquippedNameWidget>(PlayerController, EquippedNameWidgetClass))
			{
				GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, TEXT("EquippedWidget"));
				EquippedNameWidget->AddToPlayerScreen();
				EquippedNameWidgets.Add(PlayerController, EquippedNameWidget);
			}
		}
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