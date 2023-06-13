// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "InteractionInterface.h"
#include "Components/WidgetComponent.h"
#include "Door.generated.h"

class ATantrumnCharacterBase;

UCLASS()
class TANTRUMN_API ADoor : public AActor, public IInteractionInterface
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ADoor();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	virtual void InteractWithMe();
	virtual void ShowInteractionWidget() override;
	virtual void HideInteractionWidget() override;

	UFUNCTION(BlueprintCallable)
	bool GetDoorIsOpen()
	{
		return bIsOpen;
	}

	UPROPERTY(BlueprintReadWrite)
	bool bIsOpen = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UStaticMeshComponent* DoorMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UStaticMeshComponent* DoorFrameMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UWidgetComponent* InteractionWidget;

private:

	UPROPERTY(EditAnywhere)
	USceneComponent* _RootComponent;
};
