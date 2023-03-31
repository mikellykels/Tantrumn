// Fill out your copyright notice in the Description page of Project Settings.


#include "TantrumnLevelEndTrigger.h"
#include "TantrumnCharacterBase.h"

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
	if (OtherActor == Cast<ATantrumnCharacterBase>(OtherActor))
	{
		GameModeRef->PlayerReachedEnd();
	}
}
