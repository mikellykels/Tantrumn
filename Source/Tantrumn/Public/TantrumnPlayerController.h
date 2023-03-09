// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "TantrumnPlayerController.generated.h"

/**
 * 
 */
UCLASS()
class TANTRUMN_API ATantrumnPlayerController : public APlayerController
{
	GENERATED_BODY()

protected:
	void SetupInputComponent() override;

	void RequestJump();
	void RequestStopJump();

	void RequestCrouchStart();
	void RequestCrouchEnd();

	void RequestSprintStart();
	void RequestSprintEnd();

	void RequestMoveForward(float AxisValue);
	void RequestMoveRight(float AxisValue);
	void RequestLookUp(float AxisValue);
	void RequestLookRight(float AxisValue);

	// Base lookup rate, in deg/sec. Other scaling may affect final lookup rate
	UPROPERTY(EditAnywhere, Category = "Look")
	float BaseLookUpRate = 90.0f;
	
	// Base lookup rate, in deg/sec. Other scaling may affect final lookup rate
	UPROPERTY(EditAnywhere, Category = "Look")
	float BaseLookRightRate = 90.0f;

	UPROPERTY(EditAnywhere, Category = "Movement")
	float SprintSpeed = 1200.0f;
};
