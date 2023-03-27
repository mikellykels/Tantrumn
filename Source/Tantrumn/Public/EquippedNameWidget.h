// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "EquippedNameWidget.generated.h"

class UTextBlock;

/**
 * 
 */
UCLASS()
class TANTRUMN_API UEquippedNameWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UPROPERTY(meta = (BindWidget))
	UTextBlock* EquippedName;
	
};
