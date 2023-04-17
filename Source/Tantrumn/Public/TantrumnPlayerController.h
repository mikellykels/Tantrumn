// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "Sound/SoundCue.h"
#include "TantrumnPlayerController.generated.h"

class ATantrumnGameModeBase;
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

protected:
	void SetupInputComponent() override;

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

	void RequestPullObjectStart();
	void RequestPullObjectStop();

	void OnInteract();

	void OnPauseGame();

	//UPROPERTY(EditAnywhere, Category = "HUD")
	//TSubclassOf<class UUserWidget> HUDClass;

	//UPROPERTY()
	//UUserWidget* HUDWidget;

	// Base lookup rate, in deg/sec. Other scaling may affect final lookup rate
	UPROPERTY(EditAnywhere, Category = "Look")
	float BaseLookUpRate = 90.0f;
	
	// Base lookup rate, in deg/sec. Other scaling may affect final lookup rate
	UPROPERTY(EditAnywhere, Category = "Look")
	float BaseLookRightRate = 90.0f;

	UPROPERTY(EditAnywhere, Category = "Sound")
	USoundCue* JumpSound = nullptr;

	ATantrumnGameModeBase* GameModeRef;

};
