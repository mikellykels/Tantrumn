// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "TantrumnEnemyAIController.generated.h"

class APawn;

/**
 * 
 */
UCLASS()
class TANTRUMN_API ATantrumnEnemyAIController : public AAIController
{
	GENERATED_BODY()

public:
	virtual void OnPossess(APawn* InPawn) override;
	virtual void OnUnPossess() override;

	UFUNCTION()
	void OnReachedEnd();
	
};
