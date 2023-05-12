// Fill out your copyright notice in the Description page of Project Settings.


#include "TantrumnPlayerController.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "TantrumnCharacterBase.h"
#include "TantrumnGameInstance.h"
#include "TantrumnGameStateBase.h"
#include "TantrumnPlayerState.h"

#include "../TantrumnGameModeBase.h"

static TAutoConsoleVariable<bool> CVarDisplayLaunchInputDelta(
	TEXT("Tantrum.Character.Debug.DisplayLaunchInputDelta"),
	false,
	TEXT("Display Launch Input Delta"),
	ECVF_Default);

void ATantrumnPlayerController::BeginPlay()
{
	Super::BeginPlay();

	TantrumnGameState = GetWorld()->GetGameState<ATantrumnGameStateBase>();
}

//called from gamemode, so only on authority will we get these calls
void ATantrumnPlayerController::OnPossess(APawn* aPawn)
{
	Super::OnPossess(aPawn);
	UE_LOG(LogTemp, Warning, TEXT("OnPossess: %s"), *GetName());
}

void ATantrumnPlayerController::OnUnPossess()
{
	Super::OnUnPossess();
	UE_LOG(LogTemp, Warning, TEXT("OnUnPossess: %s"), *GetName());
}

void ATantrumnPlayerController::ClientDisplayCountdown_Implementation(float GameCountdownDuration)
{
	if (UTantrumnGameInstance* TantrumnGameInstance = GetWorld()->GetGameInstance<UTantrumnGameInstance>())
	{
		TantrumnGameInstance->DisplayCountdown(GameCountdownDuration, this);
	}
}

void ATantrumnPlayerController::ClientRestartGame_Implementation()
{
	if (ATantrumnPlayerState* TantrumnPlayerState = GetPlayerState<ATantrumnPlayerState>())
	{
		if (UTantrumnGameInstance* TantrumnGameInstance = GetWorld()->GetGameInstance<UTantrumnGameInstance>())
		{
			TantrumnGameInstance->RestartGame(this);
		}
	}
}

void ATantrumnPlayerController::ClientReachedEnd_Implementation()
{
	//this needs to be named better, it's just displaying the end screen
	//this will be seperate, as it will come after the montage...
	//client gets hud authority needs to replicate the montage

	if (ATantrumnCharacterBase* TantrumnCharacterBase = Cast<ATantrumnCharacterBase>(GetCharacter()))
	{
		TantrumnCharacterBase->ServerPlayCelebrateMontage();
		TantrumnCharacterBase->GetCharacterMovement()->DisableMovement();
	}

	if (UTantrumnGameInstance* TantrumnGameInstance = GetWorld()->GetGameInstance<UTantrumnGameInstance>())
	{
		//call the level complete event for the widget...
	}

	FInputModeUIOnly InputMode;
	SetInputMode(InputMode);
	SetShowMouseCursor(true);
}

void ATantrumnPlayerController::ServerRestartLevel_Implementation()
{
	//GetWorld()->ServerTravel(TEXT("?restart"));
	ATantrumnGameModeBase* TantrumnGameMode = GetWorld()->GetAuthGameMode<ATantrumnGameModeBase>();
	if (ensureMsgf(TantrumnGameMode, TEXT("ATantrumnPlayerController::ServerRestartLevel_Implementation Invalid GameMode")))
	{
		TantrumnGameMode->RestartGame();

	}
	/*RestartPlayer()
	GetWorld()->GetCurrentLevel()->GetName()
	GetWorld()->ServerTravel(TEXT("?restart"));*/
}

void ATantrumnPlayerController::ReceivedPlayer()
{
	Super::ReceivedPlayer();

	if (IsLocalController())
	{
		//if (HUDClass)
		//{
		//	HUDWidget = CreateWidget(this, HUDClass);
		//	if (HUDWidget)
		//	{
		//		HUDWidget->AddToPlayerScreen();
		//	}
		//}
	}
}

void ATantrumnPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	if (InputComponent)
	{
		// Movement
		InputComponent->BindAction(TEXT("Jump"), EInputEvent::IE_Pressed, this, &ATantrumnPlayerController::RequestJumpStart);
		InputComponent->BindAction(TEXT("Jump"), EInputEvent::IE_Released, this, &ATantrumnPlayerController::RequestJumpStop);

		InputComponent->BindAction(TEXT("Crouch"), EInputEvent::IE_Pressed, this, &ATantrumnPlayerController::RequestCrouchStart);
		InputComponent->BindAction(TEXT("Crouch"), EInputEvent::IE_Released, this, &ATantrumnPlayerController::RequestCrouchStop);

		InputComponent->BindAction(TEXT("Sprint"), EInputEvent::IE_Pressed, this, &ATantrumnPlayerController::RequestSprintStart);
		InputComponent->BindAction(TEXT("Sprint"), EInputEvent::IE_Released, this, &ATantrumnPlayerController::RequestSprintStop);

		InputComponent->BindAction(TEXT("Fire"), EInputEvent::IE_Pressed, this, &ATantrumnPlayerController::Fire);

		InputComponent->BindAxis(TEXT("MoveForward"), this, &ATantrumnPlayerController::RequestMoveForward);
		InputComponent->BindAxis(TEXT("MoveRight"), this, &ATantrumnPlayerController::RequestMoveRight);
		InputComponent->BindAxis(TEXT("LookUp"), this, &ATantrumnPlayerController::RequestLookUp);
		InputComponent->BindAxis(TEXT("LookRight"), this, &ATantrumnPlayerController::RequestLookRight);

		// Interact
		InputComponent->BindAction(TEXT("Interact"), EInputEvent::IE_Pressed, this, &ATantrumnPlayerController::OnInteract);

		// Pause
		InputComponent->BindAction(TEXT("Pause"), EInputEvent::IE_Pressed, this, &ATantrumnPlayerController::ClientPauseGame);

		// Pull/Throw
		InputComponent->BindAction(TEXT("PullObject"), EInputEvent::IE_Pressed, this, &ATantrumnPlayerController::RequestPullObjectStart);
		InputComponent->BindAction(TEXT("PullObject"), EInputEvent::IE_Released, this, &ATantrumnPlayerController::RequestPullObjectStop);

		InputComponent->BindAction(TEXT("ThrowObject"), EInputEvent::IE_Pressed, this, &ATantrumnPlayerController::RequestThrowObject);

		// Use object
		InputComponent->BindAction(TEXT("UseObject"), EInputEvent::IE_Pressed, this, &ATantrumnPlayerController::RequestUseObject);
	}
}

bool ATantrumnPlayerController::CanProcessRequest() const
{
	if (TantrumnGameState && TantrumnGameState->IsPlaying())
	{
		if (ATantrumnPlayerState* TantrumnPlayerState = GetPlayerState<ATantrumnPlayerState>())
		{
			return (TantrumnPlayerState->GetCurrentState() == EPlayerGameState::Playing);
		}
	}

	return false;
}

void ATantrumnPlayerController::RequestJumpStart()
{
	if (!CanProcessRequest())
	{
		return;
	}

	if (ATantrumnCharacterBase* TantrumnCharacterBase = Cast<ATantrumnCharacterBase>(GetCharacter()))
	{
		TantrumnCharacterBase->RequestJumpStart();
	}
}

void ATantrumnPlayerController::RequestJumpStop()
{
	if (ATantrumnCharacterBase* TantrumnCharacterBase = Cast<ATantrumnCharacterBase>(GetCharacter()))
	{
		TantrumnCharacterBase->RequestJumpStop();
	}
}

void ATantrumnPlayerController::RequestCrouchStart()
{
	if (!CanProcessRequest())
	{
		return;
	}

	if (!GetCharacter()->GetCharacterMovement()->IsMovingOnGround())
	{
		return;
	}

	if (GetCharacter())
	{
		GetCharacter()->Crouch();
	}
}

void ATantrumnPlayerController::RequestCrouchStop()
{
	if (GetCharacter())
	{
		GetCharacter()->UnCrouch();
	}
}

void ATantrumnPlayerController::RequestSprintStart()
{
	if (!CanProcessRequest())
	{
		return;
	}

	if (ATantrumnCharacterBase* TantrumnCharacterBase = Cast<ATantrumnCharacterBase>(GetCharacter()))
	{
		TantrumnCharacterBase->RequestSprintStart();
	}
}

void ATantrumnPlayerController::RequestSprintStop()
{
	if (ATantrumnCharacterBase* TantrumnCharacterBase = Cast<ATantrumnCharacterBase>(GetCharacter()))
	{
		TantrumnCharacterBase->RequestSprintEnd();
	}
}

void ATantrumnPlayerController::Fire()
{
	if (ATantrumnCharacterBase* TantrumnCharacterBase = Cast<ATantrumnCharacterBase>(GetCharacter()))
	{
		TantrumnCharacterBase->Fire();
	}
}

void ATantrumnPlayerController::RequestMoveForward(float AxisValue)
{
	if (!CanProcessRequest())
	{
		return;
	}

	if (AxisValue != 0.f)
	{
		FRotator const ControlSpaceRot = GetControlRotation();
		// transform to world space and add it
		GetPawn()->AddMovementInput(FRotationMatrix(ControlSpaceRot).GetScaledAxis(EAxis::X), AxisValue);
	}
}

void ATantrumnPlayerController::RequestMoveRight(float AxisValue)
{
	if (!CanProcessRequest())
	{
		return;
	}

	if (AxisValue != 0.f)
	{
		FRotator const ControlSpaceRot = GetControlRotation();
		// transform to world space and add it
		GetPawn()->AddMovementInput(FRotationMatrix(ControlSpaceRot).GetScaledAxis(EAxis::Y), AxisValue);
	}
}

void ATantrumnPlayerController::RequestLookUp(float AxisValue)
{
	AddPitchInput(AxisValue * BaseLookUpRate * GetWorld()->GetDeltaSeconds());
}

void ATantrumnPlayerController::RequestLookRight(float AxisValue)
{
	AddYawInput(AxisValue * BaseLookRightRate * GetWorld()->GetDeltaSeconds());
}

void ATantrumnPlayerController::RequestThrowObject()
{
	if (!CanProcessRequest())
	{
		return;
	}

	if (ATantrumnCharacterBase* TantrumnCharacterBase = Cast<ATantrumnCharacterBase>(GetCharacter()))
	{
		TantrumnCharacterBase->RequestThrowObject();
	}
}

void ATantrumnPlayerController::RequestUseObject()
{
	if (ATantrumnCharacterBase* TantrumnCharacterBase = Cast<ATantrumnCharacterBase>(GetCharacter()))
	{
		TantrumnCharacterBase->RequestUseObject();
	}
}

void ATantrumnPlayerController::RequestPullObjectStart()
{
	if (!CanProcessRequest())
	{
		return;
	}

	if (ATantrumnCharacterBase* TantrumnCharacterBase = Cast<ATantrumnCharacterBase>(GetCharacter()))
	{
		TantrumnCharacterBase->RequestPullObjectStart();
	}
}

void ATantrumnPlayerController::RequestPullObjectStop()
{
	if (ATantrumnCharacterBase* TantrumnCharacterBase = Cast<ATantrumnCharacterBase>(GetCharacter()))
	{
		TantrumnCharacterBase->RequestPullObjectStop();
	}
}

void ATantrumnPlayerController::OnInteract()
{
	if (!CanProcessRequest())
	{
		return;
	}

	if (ATantrumnCharacterBase* TantrumnCharacterBase = Cast<ATantrumnCharacterBase>(GetCharacter()))
	{
		if (TantrumnCharacterBase->Interface)
		{
			TantrumnCharacterBase->Interface->InteractWithMe();
		}
	}
}

void ATantrumnPlayerController::ClientPauseGame_Implementation()
{
	if (ATantrumnPlayerState* TantrumnPlayerState = GetPlayerState<ATantrumnPlayerState>())
	{
		if (UTantrumnGameInstance* TantrumnGameInstance = GetWorld()->GetGameInstance<UTantrumnGameInstance>())
		{
			TantrumnGameInstance->PauseGame(this);
		}
	}
	//GameModeRef->PlayerPausedGame(this);
}

void ATantrumnPlayerController::ClientDisplayEquippedWidget_Implementation()
{
	if (ATantrumnPlayerState* TantrumnPlayerState = GetPlayerState<ATantrumnPlayerState>())
	{
		if (UTantrumnGameInstance* TantrumnGameInstance = GetWorld()->GetGameInstance<UTantrumnGameInstance>())
		{
			TantrumnGameInstance->DisplayEquippedWidget(this);
		}
	}
}


