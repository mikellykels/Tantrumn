// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "TantrumnPlayerController.generated.h"

class ATantrumnGameModeBase;
class ATantrumnCharacterBase;
class ATantrumnGameStateBase;
class UTantrumnGameWidget;
class UUserWidget;

/**
 * 
 */
UCLASS()
class TANTRUMN_API ATantrumnPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	
	virtual void BeginPlay() override;
	//in local mp we need to make sure the controller has received the player in order to correctly set up the hud
	virtual void ReceivedPlayer() override;

	virtual void OnPossess(APawn* aPawn) override;
	virtual void OnUnPossess() override;

	UFUNCTION(Client, Reliable)
	void ClientDisplayCountdown(float GameCountdownDuration, TSubclassOf<UTantrumnGameWidget> InGameWidgetClass);

	UFUNCTION(Client, Reliable)
	void ClientRestartGame();

	UFUNCTION(Client, Reliable)
	void ClientReachedEnd(ATantrumnCharacterBase* TantrumnCharacter);

	UFUNCTION(Client, Reliable)
	void ClientPauseGame();

	UFUNCTION(Client, Reliable)
	void ClientDisplayEquippedWidget();

	UFUNCTION(Server, Reliable)
	void ServerRestartLevel();

	UFUNCTION(BlueprintCallable)
	void OnRetrySelected();


protected:
	void SetupInputComponent() override;

	bool CanProcessRequest() const;

	void RequestJumpStart();
	void RequestJumpStop();

	void RequestCrouchStart();
	void RequestCrouchStop();

	void RequestSprintStart();
	void RequestSprintStop();

	void Fire();

	void RequestMoveForward(float AxisValue);
	void RequestMoveRight(float AxisValue);
	void RequestLookUp(float AxisValue);
	void RequestLookRight(float AxisValue);

	void RequestThrowObject();
	void RequestUseObject();

	void RequestPullorAimObjectStart();
	void RequestPullorAimObjectStop();

	void OnInteract();

	UPROPERTY(EditAnywhere, Category = "HUD")
	TSubclassOf<UUserWidget> HUDClass;

	UPROPERTY()
	UUserWidget* HUDWidget;

	UPROPERTY()
	UTantrumnGameWidget* TantrumnGameWidget;

	// Base lookup rate, in deg/sec. Other scaling may affect final lookup rate
	UPROPERTY(EditAnywhere, Category = "Look")
	float BaseLookUpRate = 90.0f;
	
	// Base lookup rate, in deg/sec. Other scaling may affect final lookup rate
	UPROPERTY(EditAnywhere, Category = "Look")
	float BaseLookRightRate = 90.0f;

	UPROPERTY()
	ATantrumnGameStateBase* TantrumnGameState;

	ATantrumnGameModeBase* GameModeBase;
};
