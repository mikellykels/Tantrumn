// Fill out your copyright notice in the Description page of Project Settings.


#include "TantrumnGameInstance.h"
#include "TantrumnGameWidget.h"
#include "TantrumnGameStateBase.h"
#include "TantrumnPausedWidget.h"
#include "ThrowableActor.h"
#include "EquippedNameWidget.h"
#include "Components/TextBlock.h"
#include "Kismet/GameplayStatics.h"
#include "TantrumnPlayerController.h"

void UTantrumnGameInstance::DisplayCountdown(float GameCountdownDuration, ATantrumnPlayerController* TantrumnPlayerController)
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
			PlayerGameWidget->StartCountdown(GameCountdownDuration, TantrumnPlayerController);
			TantrumnGameStateBase = GetWorld()->GetGameState<ATantrumnGameStateBase>();
			if (!TantrumnGameStateBase)
			{
				GetWorld()->GameStateSetEvent.AddUObject(this, &UTantrumnGameInstance::OnGameStateSet);
			}
		}
	}
}

void UTantrumnGameInstance::OnGameStateSet(AGameStateBase* GameStateBase)
{
	TantrumnGameStateBase = Cast<ATantrumnGameStateBase>(GameStateBase);
}

void UTantrumnGameInstance::DisplayLevelComplete(ATantrumnPlayerController* TantrumnPlayerController)
{
	UTantrumnGameWidget** GameWidget = GameWidgets.Find(TantrumnPlayerController);
	if (GameWidget)
	{
		(*GameWidget)->DisplayResults();
	}
}

void UTantrumnGameInstance::DisplayPausedMenu(ATantrumnPlayerController* TantrumnPlayerController)
{
	if (!PausedWidgetClass)
	{
		return;
	}

	if (UTantrumnPausedWidget* PausedWidget = CreateWidget<UTantrumnPausedWidget>(TantrumnPlayerController, PausedWidgetClass))
	{
		UE_LOG(LogTemp, Warning, TEXT("Display menu"));
		PausedWidget->LevelPaused(TantrumnPlayerController);
		PausedWidget->AddToPlayerScreen();
		PausedWidgets.Add(TantrumnPlayerController, PausedWidget);
	}
}

void UTantrumnGameInstance::DisplayEquippedWidget(ATantrumnPlayerController* TantrumnPlayerController)
{
	if (!EquippedNameWidgetClass)
	{
		return;
	}

	if (UEquippedNameWidget* EquippedNameWidget = CreateWidget<UEquippedNameWidget>(TantrumnPlayerController, EquippedNameWidgetClass))
	{
		EquippedNameWidget->AddToPlayerScreen();
		EquippedNameWidgets.Add(TantrumnPlayerController, EquippedNameWidget);
	}
}

void UTantrumnGameInstance::DisplayEquippedName(AThrowableActor* InThrowableActor, APlayerController* PlayerController)
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

void UTantrumnGameInstance::DisplayBuffName(AThrowableActor* InThrowableActor, APlayerController* PlayerController)
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

void UTantrumnGameInstance::RemoveEquippedName(APlayerController* PlayerController)
{
	UEquippedNameWidget** EquippedNameWidget = EquippedNameWidgets.Find(PlayerController);

	if (EquippedNameWidget)
	{
		(*EquippedNameWidget)->EquippedName->SetText(FText::FromString(""));
	}
}

void UTantrumnGameInstance::RemoveBuffName(APlayerController* PlayerController)
{
	UEquippedNameWidget** EquippedNameWidget = EquippedNameWidgets.Find(PlayerController);

	if (EquippedNameWidget)
	{
		(*EquippedNameWidget)->BuffName->SetText(FText::FromString(""));
	}
}

void UTantrumnGameInstance::RestartGame(ATantrumnPlayerController* TantrumnPlayerController)
{
	UTantrumnGameWidget** GameWidget = GameWidgets.Find(TantrumnPlayerController);
	if (GameWidget)
	{
		(*GameWidget)->RemoveResults();
		//restore game input 
		TantrumnPlayerController->SetInputMode(FInputModeGameOnly());
		TantrumnPlayerController->SetShowMouseCursor(false);
	}
}

void UTantrumnGameInstance::PauseGame(ATantrumnPlayerController* TantrumnPlayerController)
{
	UGameplayStatics::SetGamePaused(GetWorld(), true);
	DisplayPausedMenu(TantrumnPlayerController);
	TantrumnPlayerController->SetInputMode(FInputModeGameAndUI());
	TantrumnPlayerController->SetShowMouseCursor(true);
}

void UTantrumnGameInstance::ResumeGame(ATantrumnPlayerController* TantrumnPlayerController)
{
	UGameplayStatics::SetGamePaused(GetWorld(), false);
	TantrumnPlayerController->SetInputMode(FInputModeGameOnly());
	TantrumnPlayerController->SetShowMouseCursor(false);

	UTantrumnPausedWidget** PausedWidget = PausedWidgets.Find(TantrumnPlayerController);
	if (PausedWidget)
	{
		(*PausedWidget)->RemoveFromParent();
		//restore game input 
		TantrumnPlayerController->SetInputMode(FInputModeGameOnly());
		TantrumnPlayerController->SetShowMouseCursor(false);
	}
}

void UTantrumnGameInstance::OnRetrySelected(ATantrumnPlayerController* TantrumnPlayerController)
{
	UTantrumnGameWidget** GameWidget = GameWidgets.Find(TantrumnPlayerController);
	if (GameWidget)
	{
		RestartGame(TantrumnPlayerController);
		TantrumnPlayerController->ServerRestartLevel();
	}
}
