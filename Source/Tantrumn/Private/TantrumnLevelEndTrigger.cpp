// Fill out your copyright notice in the Description page of Project Settings.


#include "TantrumnLevelEndTrigger.h"
#include "TantrumnCharacterBase.h"
#include "TantrumnPlayerController.h"

#include "../TantrumnGameModeBase.h"

ATantrumnLevelEndTrigger::ATantrumnLevelEndTrigger()
{
	OnActorBeginOverlap.AddDynamic(this, &ATantrumnLevelEndTrigger::OnOverlapBegin);
}

void ATantrumnLevelEndTrigger::BeginPlay()
{
	Super::BeginPlay();
	GameModeRef = GetWorld()->GetAuthGameMode<ATantrumnGameModeBase>();
}

void ATantrumnLevelEndTrigger::OnOverlapBegin(AActor* OverlappedActor, AActor* OtherActor)
{
	if (ATantrumnCharacterBase* TantrumnCharacterBase = Cast<ATantrumnCharacterBase>(OtherActor))
	{
		APlayerController* PlayerController = TantrumnCharacterBase->GetController<APlayerController>();
		GameModeRef->PlayerReachedEnd(PlayerController);
	}
}
