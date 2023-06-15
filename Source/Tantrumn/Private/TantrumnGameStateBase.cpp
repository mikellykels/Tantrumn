// Fill out your copyright notice in the Description page of Project Settings.


#include "TantrumnGameStateBase.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"
#include "TantrumnCharacterBase.h"
#include "TantrumnEnemyAIController.h"
#include "TantrumnPlayerController.h"
#include "TantrumnPlayerState.h"

void ATantrumnGameStateBase::UpdateResults(ATantrumnPlayerState* PlayerState, ATantrumnCharacterBase* TantrumnCharacter)
{
	if (!PlayerState || !TantrumnCharacter)
	{
		return;
	}

	const bool IsWinner = Results.Num() == 0;
	PlayerState->SetIsWinner(IsWinner);
	PlayerState->SetCurrentState(EPlayerGameState::Finished);

	FGameResult Result;
	Result.Name = TantrumnCharacter->GetName();
	//TODO get the actual time it took in order to post to a leaderboard/results widget
	Result.Time = 5.0f;
	Results.Add(Result);
}

//only ever called by the authority
void ATantrumnGameStateBase::OnPlayerReachedEnd(ATantrumnCharacterBase* TantrumnCharacter)
{
	ensureMsgf(HasAuthority(), TEXT("ATantrumnGameStateBase::OnPlayerReachedEnd being called from Non Authority!"));
	if (ATantrumnPlayerController* TantrumnPlayerController = TantrumnCharacter->GetController<ATantrumnPlayerController>())
	{

		TantrumnPlayerController->ClientReachedEnd(TantrumnCharacter);
		TantrumnCharacter->GetCharacterMovement()->DisableMovement();

		// SoundCue Triggers
		if (EndLevelSound && TantrumnCharacter->GetOwner())
		{
			FVector CharacterLocation = TantrumnCharacter->GetOwner()->GetActorLocation();
			UGameplayStatics::PlaySoundAtLocation(this, EndLevelSound, CharacterLocation);
		}

		ATantrumnPlayerState* PlayerState = TantrumnPlayerController->GetPlayerState<ATantrumnPlayerState>();
		UpdateResults(PlayerState, TantrumnCharacter);

		//TODO this will not work once JIP(Join In Progress) is enabled
		if (Results.Num() == PlayerArray.Num())
		{
			GameState = EGameState::GameOver;
		}
	}
	else if (ATantrumnEnemyAIController* TantrumnEnemyAIController = TantrumnCharacter->GetController<ATantrumnEnemyAIController>())
	{
		ATantrumnPlayerState* PlayerState = TantrumnEnemyAIController->GetPlayerState<ATantrumnPlayerState>();
		UpdateResults(PlayerState, TantrumnCharacter);
		TantrumnEnemyAIController->OnReachedEnd();
	}
}

void ATantrumnGameStateBase::OnPaused(ATantrumnCharacterBase* TantrumnCharacter)
{
	ensureMsgf(HasAuthority(), TEXT("ATantrumnGameStateBase::OnPaused being called from Non Authority!"));

	if (ATantrumnPlayerController* TantrumnPlayerController = TantrumnCharacter->GetController<ATantrumnPlayerController>())
	{
		TantrumnPlayerController->ClientPauseGame();
		GameState = EGameState::Paused;
	}
}

void ATantrumnGameStateBase::ClearResults()
{
	Results.Empty();
}

void ATantrumnGameStateBase::GetLifetimeReplicatedProps(TArray< FLifetimeProperty >& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	FDoRepLifetimeParams SharedParams;
	SharedParams.bIsPushBased = true;

	DOREPLIFETIME_WITH_PARAMS_FAST(ATantrumnGameStateBase, GameState, SharedParams);
	DOREPLIFETIME_WITH_PARAMS_FAST(ATantrumnGameStateBase, Results, SharedParams);
}

void ATantrumnGameStateBase::OnRep_GameState(const EGameState& OldGameState)
{
	UE_LOG(LogTemp, Warning, TEXT("OldGameState: %s"), *UEnum::GetDisplayValueAsText(OldGameState).ToString());
	UE_LOG(LogTemp, Warning, TEXT("GameState: %s"), *UEnum::GetDisplayValueAsText(GameState).ToString());
}