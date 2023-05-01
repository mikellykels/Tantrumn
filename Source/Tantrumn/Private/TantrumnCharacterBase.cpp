// Fill out your copyright notice in the Description page of Project Settings.


#include "TantrumnCharacterBase.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Components/TextBlock.h"
#include "Blueprint/UserWidget.h"
#include "DrawDebugHelpers.h"
#include "TantrumnPlayerController.h"
#include "Projectile.h"
#include "LightSwitch.h"
#include "Net/UnrealNetwork.h"
#include "ThrowableActor.h"

#include "../TantrumnGameModeBase.h"

constexpr int CVSphereCastPlayerView = 0;
constexpr int CVSphereCastActorTransform = 1;
constexpr int CVLineCastActorTransform = 2;

// Add cvars for debug
static TAutoConsoleVariable<int> CVarTraceMode(
	TEXT("Tantrum.Character.Debug.TraceMode"),
	0,
	TEXT("	0: Sphere cast PlayerView is used for direction/rotation (default).\n")
	TEXT("	1: Sphere cast using ActorTransform \n")
	TEXT("	2: Line cast using ActorTransform \n"),
	ECVF_Default);

static TAutoConsoleVariable<bool> CVarDisplayTrace(
	TEXT("Tantrum.Character.Debug.DisplayTrace"),
	false,
	TEXT("Display Trace"),
	ECVF_Default);

static TAutoConsoleVariable<bool> CVarDisplayThrowVelocity(
	TEXT("Tantrum.Character.Debug.DisplayThrowVelocity"),
	false,
	TEXT("Display Throw Velocity"),
	ECVF_Default);

// Sets default values
ATantrumnCharacterBase::ATantrumnCharacterBase()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;

	ProjectileSpawnPoint = CreateDefaultSubobject<USceneComponent>(TEXT("Spawn Point"));
	ProjectileSpawnPoint->SetupAttachment(RootComponent);

	InteractionBox = CreateDefaultSubobject<UBoxComponent>(TEXT("Interaction Box"));
	InteractionBox->SetupAttachment(RootComponent);
}

void ATantrumnCharacterBase::GetLifetimeReplicatedProps(TArray< FLifetimeProperty >& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	FDoRepLifetimeParams SharedParams;
	SharedParams.bIsPushBased = true;
	SharedParams.Condition = COND_SkipOwner;

	DOREPLIFETIME_WITH_PARAMS_FAST(ATantrumnCharacterBase, CharacterThrowState, SharedParams);

	//DOREPLIFETIME(ATantrumnCharacterBase, CharacterThrowState);
}

// Called when the game starts or when spawned
void ATantrumnCharacterBase::BeginPlay()
{
	Super::BeginPlay();

	EffectCooldown = DefaultEffectCooldown;

	if (GetCharacterMovement())
	{
		MaxWalkSpeed = GetCharacterMovement()->MaxWalkSpeed;
	}
	
	InteractionBox->OnComponentBeginOverlap.AddDynamic(this, &ATantrumnCharacterBase::OnBoxBeginOverlap);
	InteractionBox->OnComponentEndOverlap.AddDynamic(this, &ATantrumnCharacterBase::OnBoxEndOverlap);

	GameModeRef = GetWorld()->GetAuthGameMode<ATantrumnGameModeBase>();
}

void ATantrumnCharacterBase::ApplyPowerEffect()
{
	ThrowSpeed *= 10;
	TArray<AActor*> ThrowableActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AThrowableActor::StaticClass(), ThrowableActors);

	for (AActor* Throwable : ThrowableActors)
	{
		AThrowableActor* ThrowableActorCast = Cast<AThrowableActor>(Throwable);
		if (ThrowableActorCast)
		{
			ThrowableActorCast->ProjectileMovementComponent->MaxSpeed = 10000;
		}
	}
}

void ATantrumnCharacterBase::EndPowerEffect()
{
	ThrowSpeed /= 10;
	TArray<AActor*> ThrowableActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AThrowableActor::StaticClass(), ThrowableActors);

	for (AActor* Throwable : ThrowableActors)
	{
		AThrowableActor* ThrowableActorCast = Cast<AThrowableActor>(Throwable);
		if (ThrowableActorCast)
		{
			ThrowableActorCast->ProjectileMovementComponent->MaxSpeed = ThrowSpeed;
		}
	}
}

// Called every frame
void ATantrumnCharacterBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!IsLocallyControlled())
	{
		return;
	}
	/*if (!HasAuthority())
	{
		return;
	}*/
	//this only needs to be done on the owner
	//the pull needs to be done on the server
	//as that's the owner of the objects we are pulling
	UpdateStun();
	if (bIsStunned)
	{
		return;
	}

	if (bIsPlayerBeingRescued)
	{
		UpdateRescue(DeltaTime);
		return;
	}

	if (bIsUnderEffect)
	{
		if (EffectCooldown > 0)
		{
			EffectCooldown -= DeltaTime;
		}
		else
		{
			bIsUnderEffect = false;
			EffectCooldown = DefaultEffectCooldown;
			EndEffect();
		}
	}

	if (CharacterThrowState == ECharacterThrowState::Throwing)
	{
		if (UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance())
		{
			if (UAnimMontage* CurrentAnimMontage = AnimInstance->GetCurrentActiveMontage())
			{
				//const float PlayRate = AnimInstance->GetCurveValue(TEXT("ThrowCurve"));
				AnimInstance->Montage_SetPlayRate(CurrentAnimMontage, 1.0f);
			}
		}
	}
	else if (CharacterThrowState == ECharacterThrowState::None || CharacterThrowState == ECharacterThrowState::RequestingPull)
	{
		switch (CVarTraceMode->GetInt())
		{
		case CVSphereCastPlayerView:
			SphereCastPlayerView();
			break;
		case CVSphereCastActorTransform:
			SphereCastActorTransform();
			break;
		case CVLineCastActorTransform:
			LineCastActorTransform();
			break;
		default:
			SphereCastPlayerView();
			break;
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
		else
		{
			// SoundCue Triggers
			if (HeavyLandSound && GetOwner())
			{
				FVector CharacterLocation = GetOwner()->GetActorLocation();
				UGameplayStatics::PlaySoundAtLocation(this, HeavyLandSound, CharacterLocation);
			}
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

void ATantrumnCharacterBase::OnMovementModeChanged(EMovementMode PrevMovementMode, uint8 PreviousCustomMode)
{
	if (!bIsPlayerBeingRescued && (PrevMovementMode == MOVE_Walking && GetCharacterMovement()->MovementMode == MOVE_Falling))
	{
		LastGroundPosition = GetActorLocation() + (GetActorForwardVector() * -100.0f) + (GetActorUpVector() * 100.0f);
	}

	Super::OnMovementModeChanged(PrevMovementMode, PreviousCustomMode);
}

void ATantrumnCharacterBase::FellOutOfWorld(const UDamageType& dmgType)
{
	FallOutOfWorldPosition = GetActorLocation();
	StartRescue();
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

void ATantrumnCharacterBase::RequestJumpStart()
{
	if (!bIsStunned)
	{
		GetCharacterMovement()->JumpZVelocity = JumpHeight;
		GetController()->GetCharacter()->Jump();
	}
}

void ATantrumnCharacterBase::RequestJumpStop()
{
	GetController()->GetCharacter()->StopJumping();
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
	GetMesh();
}

void ATantrumnCharacterBase::UpdateStun()
{
	if (bIsStunned)
	{
		bIsStunned = (FApp::GetCurrentTime() - StunBeginTimestamp) < StunTime;
		if (!bIsStunned)
		{
			OnStunEnd();
		}
	}
}

void ATantrumnCharacterBase::OnStunEnd()
{
	StunBeginTimestamp = 0.0f;
	StunTime = 0.0f;
}

void ATantrumnCharacterBase::UpdateRescue(float DeltaTime)
{
	CurrentRescueTime += DeltaTime;
	float Alpha = FMath::Clamp(CurrentRescueTime / TimeToRescuePlayer, 0.0f, 1.0f);
	FVector NewPlayerLocation = FMath::Lerp(FallOutOfWorldPosition, LastGroundPosition, Alpha);
	SetActorLocation(NewPlayerLocation);

	if (Alpha >= 1.0f)
	{
		EndRescue();
	}
}

void ATantrumnCharacterBase::StartRescue()
{
	bIsPlayerBeingRescued = true;
	CurrentRescueTime = 0.0f;
	GetCharacterMovement()->Deactivate();
	SetActorEnableCollision(false);
}

void ATantrumnCharacterBase::EndRescue()
{
	GetCharacterMovement()->Activate();
	SetActorEnableCollision(true);
	bIsPlayerBeingRescued = false;
	CurrentRescueTime = 0.0f;
}

void ATantrumnCharacterBase::Fire()
{
	FVector Location = ProjectileSpawnPoint->GetComponentLocation();
	FRotator Rotation = ProjectileSpawnPoint->GetComponentRotation();

	GetWorld()->SpawnActor<AProjectile>(ProjectileClass, Location, Rotation);

	ATantrumnPlayerController* TantrumnPlayerController = GetController<ATantrumnPlayerController>();

	if (TantrumnPlayerController)
	{
		TantrumnPlayerController->PlayDynamicForceFeedback(1.0f, 0.1f, false, true, false, true);
	}
}

void ATantrumnCharacterBase::RequestThrowObject()
{
	if (CanThrowObject())
	{
		if (PlayThrowMontage())
		{
			CharacterThrowState = ECharacterThrowState::Throwing;
			ServerRequestThrowObject();
			bIsThrowableActorAttached = false;

			APlayerController* PlayerController = GetController<APlayerController>();
			GameModeRef->RemoveEquippedName(PlayerController);
		}
		else
		{
			ResetThrowableObject();
		}
	}
}

void ATantrumnCharacterBase::RequestPullObjectStart()
{
	if (!bIsStunned && CharacterThrowState == ECharacterThrowState::None)
	{
		CharacterThrowState = ECharacterThrowState::RequestingPull;
		ServerRequestPullObject(true);
	}
}

void ATantrumnCharacterBase::RequestPullObjectStop()
{
	// If we were pulling an object, drop it
	if (CharacterThrowState == ECharacterThrowState::RequestingPull)
	{
		CharacterThrowState = ECharacterThrowState::None;
		ServerRequestPullObject(false);
		//ResetThrowableObject();
	}
}

void ATantrumnCharacterBase::ResetThrowableObject()
{
	// Drop object
	if (ThrowableActor)
	{
		ThrowableActor->Drop();
	}
	CharacterThrowState = ECharacterThrowState::None;
	ThrowableActor = nullptr;
}

void ATantrumnCharacterBase::RequestUseObject()
{
	if (!bIsUnderEffect && ThrowableActor->GetEffectType() != EEffectType::None)
	{
		CharacterThrowState = ECharacterThrowState::None;
		APlayerController* PlayerController = GetController<APlayerController>();
		GameModeRef->RemoveEquippedName(PlayerController);
		GameModeRef->DisplayBuffName(ThrowableActor, PlayerController);

		ApplyEffect_Implementation(ThrowableActor->GetEffectType(), true);
		ThrowableActor->Destroy();
		ResetThrowableObject();
	}
}

void ATantrumnCharacterBase::OnThrowableAttached(AThrowableActor* InThrowableActor)
{
	CharacterThrowState = ECharacterThrowState::Attached;
	bIsThrowableActorAttached = true;
	ThrowableActor = InThrowableActor;
	MoveIgnoreActorAdd(ThrowableActor);
	ClientThrowableAttached(InThrowableActor);

	APlayerController* PlayerController = GetController<APlayerController>();
	GameModeRef->DisplayEquippedName(ThrowableActor, PlayerController);
}

void ATantrumnCharacterBase::SphereCastPlayerView()
{
	FVector Location;
	FRotator Rotation;
	GetController()->GetPlayerViewPoint(Location, Rotation);
	const FVector PlayerViewForward = Rotation.Vector();
	const float AdditionalDistance = (Location - GetActorLocation()).Size();
	FVector EndPos = Location + (PlayerViewForward * (1000.0f + AdditionalDistance));

	const FVector CharacterForward = GetActorForwardVector();
	const float DotResult = FVector::DotProduct(PlayerViewForward, CharacterForward);
	// Prevent picking up objects behind us, this is when the camera is looking directly at the character's front side
	if (DotResult < -0.23f)
	{
		if (ThrowableActor)
		{
			ThrowableActor->ToggleHighlight(false);
			ThrowableActor = nullptr;
		}
		return;
	}

	FHitResult HitResult;
	EDrawDebugTrace::Type DebugTrace = CVarDisplayTrace->GetBool() ? EDrawDebugTrace::ForOneFrame : EDrawDebugTrace::None;
	TArray<AActor*> ActorsToIgnore;
	ActorsToIgnore.Add(this);

	//bool UKismetSystemLibrary::SphereTraceSingle(const UObject* WorldContextObject, const FVector Start, const FVector End, float Radius, ETraceTypeQuery TraceChannel, bool bTraceComplex, const TArray<AActor*>& ActorsToIgnore, EDrawDebugTrace::Type DrawDebugType, FHitResult& OutHit, bool bIgnoreSelf, FLinearColor TraceColor, FLinearColor TraceHitColor, float DrawTime)
	UKismetSystemLibrary::SphereTraceSingle(GetWorld(), Location, EndPos, 70.0f, UEngineTypes::ConvertToTraceType(ECollisionChannel::ECC_Visibility), false, ActorsToIgnore, DebugTrace, HitResult, true);
	ProcessTraceResult(HitResult);

#if ENABLE_DRAW_DEBUG
	if (CVarDisplayTrace->GetBool())
	{
		static float FovDeg = 90.0f;
		DrawDebugCamera(GetWorld(), Location, Rotation, FovDeg);
		DrawDebugLine(GetWorld(), Location, EndPos, HitResult.bBlockingHit ? FColor::Red : FColor::White);
		DrawDebugPoint(GetWorld(), EndPos, 70.0f, HitResult.bBlockingHit ? FColor::Red : FColor::White);
	}
#endif

}

void ATantrumnCharacterBase::SphereCastActorTransform()
{
	FVector StartPos = GetActorLocation();
	FVector EndPos = StartPos + (GetActorForwardVector() * 1000.0f);

	// Sphere trace
	EDrawDebugTrace::Type DebugTrace = CVarDisplayTrace->GetBool() ? EDrawDebugTrace::ForOneFrame : EDrawDebugTrace::None;
	FHitResult HitResult;
	UKismetSystemLibrary::SphereTraceSingle(GetWorld(), StartPos, EndPos, 70.0f, UEngineTypes::ConvertToTraceType(ECollisionChannel::ECC_Visibility), false, TArray<AActor*>(), DebugTrace, HitResult, true);
	ProcessTraceResult(HitResult);
}

void ATantrumnCharacterBase::LineCastActorTransform()
{
	FVector StartPos = GetActorLocation();
	FVector EndPos = StartPos + (GetActorForwardVector() * 1000.0f);
	FHitResult HitResult;
	GetWorld() ? GetWorld()->LineTraceSingleByChannel(HitResult, StartPos, EndPos, ECollisionChannel::ECC_Visibility) : false;
#if ENABLE_DRAW_DEBUG
	if (CVarDisplayTrace->GetBool())
	{
		DrawDebugLine(GetWorld(), StartPos, EndPos, HitResult.bBlockingHit ? FColor::Red : FColor::White, false);
	}
#endif
	ProcessTraceResult(HitResult);
}

void ATantrumnCharacterBase::ProcessTraceResult(const FHitResult& HitResult)
{
	// Check if there was an existing throwable actor
	// Remove the highlight to avoid wrong feedback
	AThrowableActor* HitThrowableActor = HitResult.bBlockingHit ? Cast<AThrowableActor>(HitResult.GetActor()) : nullptr;
	const bool IsSameActor = (ThrowableActor == HitThrowableActor);
	const bool IsValidTarget = HitThrowableActor && HitThrowableActor->IsIdle();

	// Clean up old actor
	if (ThrowableActor)
	{
		if (!IsValidTarget || !IsSameActor)
		{
			ThrowableActor->ToggleHighlight(false);
			ThrowableActor = nullptr;
		}
	}

	if (!IsValidTarget)
	{
		return;
	}

	if (IsValidTarget)
	{
		if (!IsSameActor)
		{
			ThrowableActor = HitThrowableActor;
			ThrowableActor->ToggleHighlight(true);
		}
	}

	if (CharacterThrowState == ECharacterThrowState::RequestingPull)
	{
		// Don't allow for pulling objects while running/jogging
		if (GetVelocity().SizeSquared() < 100.0f)
		{
			ServerPullObject(ThrowableActor);
			ThrowableActor->ToggleHighlight(false);
			//if (ThrowableActor && ThrowableActor->Pull(this))
			//{
			//	CharacterThrowState = ECharacterThrowState::Pulling;
			//	ThrowableActor = nullptr;
			//}
		}
	}
}

void ATantrumnCharacterBase::ServerPullObject_Implementation(AThrowableActor* InThrowableActor)
{
	if (InThrowableActor && InThrowableActor->Pull(this))
	{
		CharacterThrowState = ECharacterThrowState::Pulling;
		ThrowableActor = InThrowableActor;
		ThrowableActor->ToggleHighlight(false);
	}
}

void ATantrumnCharacterBase::ServerRequestPullObject_Implementation(bool bIsPulling)
{
	CharacterThrowState = bIsPulling ? ECharacterThrowState::RequestingPull : ECharacterThrowState::None;
}

void ATantrumnCharacterBase::ServerRequestThrowObject_Implementation()
{
	MulticastRequestThrowObject();
}

void ATantrumnCharacterBase::MulticastRequestThrowObject_Implementation()
{
	if (IsLocallyControlled())
	{
		return;
	}

	PlayThrowMontage();
	CharacterThrowState = ECharacterThrowState::Throwing;
}

void ATantrumnCharacterBase::ClientThrowableAttached_Implementation(AThrowableActor* InThrowableActor)
{
	CharacterThrowState = ECharacterThrowState::Attached;
	ThrowableActor = InThrowableActor;
	MoveIgnoreActorAdd(ThrowableActor);
}

void ATantrumnCharacterBase::ServerBeginThrow_Implementation()
{
	//ignore collisions otherwise the throwable object hits the player capsule and doesn't travel in the desired direction
	if (ThrowableActor->GetRootComponent())
	{
		UPrimitiveComponent* RootPrimitiveComponent = Cast<UPrimitiveComponent>(ThrowableActor->GetRootComponent());
		if (RootPrimitiveComponent)
		{
			RootPrimitiveComponent->IgnoreActorWhenMoving(this, true);
		}
	}
	//const FVector& Direction = GetMesh()->GetSocketRotation(TEXT("ObjectAttach")).Vector() * -ThrowSpeed;
	const FVector& Direction = GetActorForwardVector() * ThrowSpeed;
	ThrowableActor->Launch(Direction);

	if (CVarDisplayThrowVelocity->GetBool())
	{
		const FVector& Start = GetMesh()->GetSocketLocation(TEXT("hand_rSocket"));
		DrawDebugLine(GetWorld(), Start, Start + Direction, FColor::Red, false, 5.0f);
	}
}

void ATantrumnCharacterBase::ServerFinishThrow_Implementation()
{
	//put all this in a function that runs on the server
	CharacterThrowState = ECharacterThrowState::None;
	//this only happened on the locally controlled actor
	MoveIgnoreActorRemove(ThrowableActor);
	if (ThrowableActor->GetRootComponent())
	{
		UPrimitiveComponent* RootPrimitiveComponent = Cast<UPrimitiveComponent>(ThrowableActor->GetRootComponent());
		if (RootPrimitiveComponent)
		{
			RootPrimitiveComponent->IgnoreActorWhenMoving(this, false);
		}
	}
	ThrowableActor = nullptr;
}


void ATantrumnCharacterBase::OnBoxBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
		if (OtherActor->GetClass()->ImplementsInterface(UInteractionInterface::StaticClass()))
		{
			APlayerController* PlayerController = GetController<APlayerController>();
			TArray<AActor*> OverlappingActors;
			GetOverlappingActors(OverlappingActors);

			if (OverlappingActors.Num() > 0)
			{
				AActor* ClosestActor = OverlappingActors[0];

				for (auto CurrentActor : OverlappingActors)
				{
					if (GetDistanceTo(CurrentActor) < GetDistanceTo(ClosestActor))
					{
						ClosestActor = CurrentActor;
					}
				}

				if (Interface)
				{
					Interface->HideInteractionWidget();
				}

				Interface = Cast<IInteractionInterface>(ClosestActor);
				if (Interface)
				{
					GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, TEXT("Show Interaction"));
					Interface->ShowInteractionWidget();
				}
			}
		}
}

void ATantrumnCharacterBase::OnBoxEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (Interface)
	{
		Interface->HideInteractionWidget();
		Interface = nullptr;
	}
}

void ATantrumnCharacterBase::ApplyEffect_Implementation(EEffectType EffectType, bool bIsBuff)
{
	if (bIsUnderEffect)
	{
		return;
	}

	CurrentEffect = EffectType;
	bIsUnderEffect = true;
	bIsEffectBuff = bIsBuff;

	switch (CurrentEffect)
	{
		case EEffectType::Speed:
			bIsEffectBuff ? SprintSpeed *= 2 : GetCharacterMovement()->DisableMovement();
			break;
		case EEffectType::Jump:
			bIsEffectBuff ? JumpHeight *= 3 : GetCharacterMovement()->DisableMovement();
			break;
		case EEffectType::Power:
			if (bIsEffectBuff)
			{
				ApplyPowerEffect();
			}
			else
			{
				GetCharacterMovement()->DisableMovement();
			}
			break;
		default:
			break;
	}
}

void ATantrumnCharacterBase::EndEffect()
{
	bIsUnderEffect = false;
	APlayerController* PlayerController = GetController<APlayerController>();
	GameModeRef->RemoveBuffName(PlayerController);

	switch (CurrentEffect)
	{
		case EEffectType::Speed :
			bIsEffectBuff ? SprintSpeed /= 2, RequestSprintEnd() : GetCharacterMovement()->SetMovementMode(MOVE_Walking);
			break;
		case EEffectType::Jump:
			bIsEffectBuff ? JumpHeight /=3, RequestJumpStop() : GetCharacterMovement()->DisableMovement();
			break;
		case EEffectType::Power:
			if (bIsEffectBuff)
			{
				EndPowerEffect();
			}
			else
			{
				GetCharacterMovement()->DisableMovement();
			}
			break;
		default:
			break;
	}
}

bool ATantrumnCharacterBase::PlayThrowMontage()
{
	const float PlayRate = 1.0f;
	bool bPlayedSuccessfully = PlayAnimMontage(ThrowMontage, PlayRate) > 0.0f;
	if (bPlayedSuccessfully)
	{
		if (IsLocallyControlled())
		{
			UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();

			if (!BlendingOutDelegate.IsBound())
			{
				BlendingOutDelegate.BindUObject(this, &ATantrumnCharacterBase::OnMontageBlendingOut);
			}
			AnimInstance->Montage_SetBlendingOutDelegate(BlendingOutDelegate, ThrowMontage);

			if (!MontageEndedDelegate.IsBound())
			{
				MontageEndedDelegate.BindUObject(this, &ATantrumnCharacterBase::OnMontageEnded);
			}
			AnimInstance->Montage_SetEndDelegate(MontageEndedDelegate, ThrowMontage);

			AnimInstance->OnPlayMontageNotifyBegin.AddDynamic(this, &ATantrumnCharacterBase::OnNotifyBeginRecieved);
			AnimInstance->OnPlayMontageNotifyEnd.AddDynamic(this, &ATantrumnCharacterBase::OnNotifyEndRecieved);
		}
	}

	return bPlayedSuccessfully;
}

void ATantrumnCharacterBase::UnbindMontage()
{
	if (IsLocallyControlled())
	{
		if (UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance())
		{
			AnimInstance->OnPlayMontageNotifyBegin.RemoveDynamic(this, &ATantrumnCharacterBase::OnNotifyBeginRecieved);
			AnimInstance->OnPlayMontageNotifyEnd.RemoveDynamic(this, &ATantrumnCharacterBase::OnNotifyEndRecieved);
		}
	}
}

void ATantrumnCharacterBase::OnMontageBlendingOut(UAnimMontage* Montage, bool bInterrupted)
{

}

void ATantrumnCharacterBase::OnMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	if (IsLocallyControlled())
	{
		UnbindMontage();
	}
	CharacterThrowState = ECharacterThrowState::None;
	ServerFinishThrow();
	ThrowableActor = nullptr;
}

void ATantrumnCharacterBase::OnNotifyBeginRecieved(FName NotifyName, const FBranchingPointNotifyPayload& BranchingPointNotifyPayload)
{
	ServerBeginThrow();
}

void ATantrumnCharacterBase::OnNotifyEndRecieved(FName NotifyName, const FBranchingPointNotifyPayload& BranchingPointNotifyPayload)
{

}

void ATantrumnCharacterBase::OnRep_CharacterThrowState(const ECharacterThrowState& OldCharacterThrowState)
{
	if (CharacterThrowState != OldCharacterThrowState)
	{
		UE_LOG(LogTemp, Warning, TEXT("OldThrowState: %s"), *UEnum::GetDisplayValueAsText(OldCharacterThrowState).ToString());
		UE_LOG(LogTemp, Warning, TEXT("CharacterThrowState: %s"), *UEnum::GetDisplayValueAsText(CharacterThrowState).ToString());
	}
}
