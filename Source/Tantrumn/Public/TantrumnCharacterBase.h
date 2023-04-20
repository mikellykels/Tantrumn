// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Components/BoxComponent.h"
#include "InteractionInterface.h"
#include "GetNameInterface.h"
#include "ThrowableActor.h"
#include "EquippedNameWidget.h"
#include "Sound/SoundCue.h"
#include "TantrumnCharacterBase.generated.h"

class UUserWidget;

UENUM(BlueprintType)
enum class ECharacterThrowState : uint8
{
	None           UMETA(DisplayName = "None"),
	RequestingPull UMETA(DisplayName = "RequestingPull"),
	Pulling        UMETA(DisplayName = "Pulling"),
	Attached       UMETA(DisplayName = "Attached"),
	Throwing       UMETA(DisplayName = "Throwing"),
};

UCLASS()
class TANTRUMN_API ATantrumnCharacterBase : public ACharacter, public IInteractionInterface
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	ATantrumnCharacterBase();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere, Category = "Movement")
	float SprintSpeed = 1200.0f;

	UPROPERTY(EditAnywhere, Category = "Movement")
	float JumpHeight = 350.0f;

	UPROPERTY(EditAnywhere, Category = "Fall Impact")
	float MinImpactSpeed = 600.0f;

	UPROPERTY(EditAnywhere, Category = "Fall Impact")
	float MaxImpactSpeed = 1600.0f;

	UPROPERTY(EditAnywhere, Category = "Fall Impact")
	float MinStunTime = 1.0f;

	UPROPERTY(EditAnywhere, Category = "Fall Impact")
	float MaxStunTime = 1.0f;

	float StunTime = 0.0f;
	float StunBeginTimestamp = 0.0f;

	bool bIsStunned = false;
	bool bIsSprinting = false;
	bool bIsThrowableActorAttached = false;

	float MaxWalkSpeed = 0.0f;

	USoundCue* HeavyLandSound = nullptr;

	void OnStunBegin(float StunRatio);
	void OnStunEnd();
	void UpdateStun();

	bool PlayThrowMontage();

	void UnbindMontage();

	UFUNCTION()
	void OnMontageBlendingOut(UAnimMontage* Montage, bool bInterrupted);

	UFUNCTION()
	void OnMontageEnded(UAnimMontage* Montage, bool bInterrupted);

	UFUNCTION()
	void OnNotifyBeginRecieved(FName NotifyName, const FBranchingPointNotifyPayload& BranchingPointNotifyPayload);

	UFUNCTION()
	void OnNotifyEndRecieved(FName NotifyName, const FBranchingPointNotifyPayload& BranchingPointNotifyPayload);

	UPROPERTY(VisibleAnywhere, Category = "Throw")
	ECharacterThrowState CharacterThrowState = ECharacterThrowState::None;

	UPROPERTY(EditAnywhere, Category = "Throw", meta = (ClampMin = "0.0", Unit = "ms"))
	float ThrowSpeed = 2000.0f;

	UPROPERTY(EditAnywhere, Category = "Animation")
	UAnimMontage* ThrowMontage = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UBoxComponent* InteractionBox;

	void DisplayEquippedWidget();
	void DisplayEquippedName();
	void RemoveEquippedName();

	void ApplyPowerEffect();
	void EndPowerEffect();

	FOnMontageBlendingOutStarted BlendingOutDelegate;
	FOnMontageEnded MontageEndedDelegate;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	virtual void Landed(const FHitResult& Hit) override;

	void RequestSprintStart();
	void RequestSprintEnd();

	void RequestJumpStart();
	void RequestJumpStop();

	void Fire();

	void RequestThrowObject();
	void RequestPullObjectStart();
	void RequestPullObjectStop();
	void ResetThrowableObject();

	void RequestUseObject();

	void OnThrowableAttached(AThrowableActor* InThrowableActor);

	void SphereCastPlayerView();

	void SphereCastActorTransform();

	void LineCastActorTransform();

	void ProcessTraceResult(const FHitResult& HitResult);

	bool CanThrowObject() const 
	{ 
		return CharacterThrowState == ECharacterThrowState::Attached || CharacterThrowState == ECharacterThrowState::Pulling;
	}

	UFUNCTION(BlueprintPure)
	bool IsPullingObject() const 
	{ 
		return CharacterThrowState == ECharacterThrowState::RequestingPull || CharacterThrowState == ECharacterThrowState::Pulling; 
	}

	UFUNCTION(BlueprintPure)
	ECharacterThrowState GetCharacterThrowState() const 
	{ 
		return CharacterThrowState; 
	}

	UFUNCTION(BlueprintPure)
	bool IsStunned() const 
	{ 
		return bIsStunned; 
	}

	UFUNCTION(BlueprintPure)
	bool IsThrowableActorAttached() const
	{
		return bIsThrowableActorAttached;
	}

	IInteractionInterface* Interface = nullptr;
	IGetNameInterface* GetNameInterface = nullptr;

private:
	UPROPERTY()
	AThrowableActor* ThrowableActor;

	UPROPERTY(EditAnywhere, Category = "HUD")
	TSubclassOf<UUserWidget> EquippedNameWidgetClass = nullptr;

	UPROPERTY()
	UEquippedNameWidget* EquippedNameWidget = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	USceneComponent* ProjectileSpawnPoint;

	UPROPERTY(EditDefaultsOnly, Category = "Combat")
	TSubclassOf<class AProjectile> ProjectileClass;

	UFUNCTION()
	void OnBoxBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnBoxEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	void ApplyEffect_Implementation(EEffectType EffectType, bool bIsBuff) override;

	void EndEffect();

	bool bIsUnderEffect = false;
	bool bIsEffectBuff = false;

	float DefaultEffectCooldown = 10.0f;
	float EffectCooldown = 0.0f;

	EEffectType CurrentEffect = EEffectType::None;
};
