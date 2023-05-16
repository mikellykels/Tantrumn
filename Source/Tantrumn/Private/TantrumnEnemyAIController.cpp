// Fill out your copyright notice in the Description page of Project Settings.


#include "TantrumnEnemyAIController.h"
#include "TantrumnCharacterBase.h"
#include "TantrumnPlayerState.h"

void ATantrumnEnemyAIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	if (ATantrumnCharacterBase* TantrumnCharacterBase = Cast<ATantrumnCharacterBase>(InPawn))
	{
		if (ATantrumnPlayerState* TantrumnPlayerState = GetPlayerState<ATantrumnPlayerState>())
		{
			TantrumnPlayerState->SetCurrentState(EPlayerGameState::Waiting);
		}
	}
}

void ATantrumnEnemyAIController::OnUnPossess()
{
	Super::OnUnPossess();
}

void ATantrumnEnemyAIController::OnReachedEnd()
{
	if (ATantrumnCharacterBase* TantrumnCharacterBase = Cast<ATantrumnCharacterBase>(GetCharacter()))
	{
		TantrumnCharacterBase->ServerPlayCelebrateMontage();
	}
}
