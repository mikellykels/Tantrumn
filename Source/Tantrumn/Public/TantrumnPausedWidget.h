// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "TantrumnPausedWidget.generated.h"

class UButton;
/**
 * 
 */
UCLASS(Abstract)
class TANTRUMN_API UTantrumnPausedWidget : public UUserWidget
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintImplementableEvent)
	void LevelPaused(class ATantrumnGameModeBase* GameMode);
};
