// Fill out your copyright notice in the Description page of Project Settings.


#include "TantrumnCharacterBase.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "TantrumnPlayerController.h"

// Sets default values
ATantrumnCharacterBase::ATantrumnCharacterBase()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void ATantrumnCharacterBase::BeginPlay()
{
	Super::BeginPlay();
	if (GetCharacterMovement())
	{
		MaxWalkSpeed = GetCharacterMovement()->MaxWalkSpeed;
	}
	
}

// Called every frame
void ATantrumnCharacterBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	if (bIsStunned)
	{
		bIsStunned = (FApp::GetCurrentTime() - StunBeginTimestamp) < StunTime;
		if (!bIsStunned)
		{
			OnStunEnd();
		}
	}

}

// Called to bind functionality to input
void ATantrumnCharacterBase::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

void ATantrumnCharacterBase::Landed(const FHitResult& Hit)
{
	Super::Landed(Hit);

	// custom landed code
	ATantrumnPlayerController* TantrumnPlayerController = GetController<ATantrumnPlayerController>();
	if (TantrumnPlayerController)
	{
		const float FallImpactSpeed = FMath::Abs(GetVelocity().Z);
		if (FallImpactSpeed < MinImpactSpeed)
		{
			// nothing to do, very light fall
			return;
		}

		const float DeltaImpact = MaxImpactSpeed - MinImpactSpeed;
		const float FallRatio = FMath::Clamp((FallImpactSpeed - MinImpactSpeed) / DeltaImpact, 0.0f, 1.0f);
		const bool bAffectSmall = FallRatio <= 0.5;
		const bool bAffectLarge = FallRatio > 0.5;
		TantrumnPlayerController->PlayDynamicForceFeedback(FallRatio, 0.5f, bAffectLarge, bAffectSmall, bAffectLarge, bAffectSmall);
		if (bAffectLarge)
		{
			OnStunBegin(FallRatio);
		}
	}
}

void ATantrumnCharacterBase::RequestSprintStart()
{
	if (!bIsStunned)
	{
		GetCharacterMovement()->MaxWalkSpeed = SprintSpeed;
	}
	bIsSprinting = true;
}

void ATantrumnCharacterBase::RequestSprintEnd()
{
	GetCharacterMovement()->MaxWalkSpeed = MaxWalkSpeed;
}

void ATantrumnCharacterBase::OnStunBegin(float StunRatio)
{
	if (bIsStunned)
	{
		// for now just exit early, alternative option would be to add the stun time
		return;
	}

	const float StunDelt = MaxStunTime - MinStunTime;
	StunTime = MinStunTime + (StunRatio * StunDelt);
	StunBeginTimestamp = FApp::GetCurrentTime();
	bIsStunned = true;
	if (bIsSprinting)
	{
		RequestSprintEnd();
	}
}

void ATantrumnCharacterBase::OnStunEnd()
{
	StunBeginTimestamp = 0.0f;
	StunTime = 0.0f;
}
