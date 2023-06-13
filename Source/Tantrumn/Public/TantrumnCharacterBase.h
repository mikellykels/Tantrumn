// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Components/BoxComponent.h"
#include "InteractionInterface.h"
#include "Sound/SoundCue.h"
#include "TantrumnCharacterBase.generated.h"

class AThrowableActor;
class ATantrumnGameModeBase;

UENUM(BlueprintType)
enum class ECharacterThrowState : uint8
{
	None           UMETA(DisplayName = "None"),
	RequestingPull UMETA(DisplayName = "RequestingPull"),
	Pulling        UMETA(DisplayName = "Pulling"),
	Attached       UMETA(DisplayName = "Attached"),
	Throwing       UMETA(DisplayName = "Throwing"),
	Aiming         UMETA(DisplayName = "Aiming"),
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
	float MaxStunTime = 5.0f;

	float StunTime = 0.0f;
	float CurrentStunTimer = 0.0f;

	bool bIsStunned = false;
	bool bIsSprinting = false;

	float MaxWalkSpeed = 0.0f;

	USoundCue* HeavyLandSound = nullptr;

	void OnStunBegin(float StunRatio);
	void OnStunEnd();
	void UpdateStun(float DeltaTime);

	void StartRescue();
	void UpdateRescue(float DeltaTime);
	void EndRescue();


	bool PlayThrowMontage();
	bool PlayCelebrateMontage();

	UFUNCTION(NetMulticast, Reliable)
	void MulticastPlayCelebrateMontage();

	void UpdateThrowMontagePlayRate();
	void UnbindMontage();

	UFUNCTION()
	void OnMontageBlendingOut(UAnimMontage* Montage, bool bInterrupted);

	UFUNCTION()
	void OnMontageEnded(UAnimMontage* Montage, bool bInterrupted);

	UFUNCTION()
	void OnNotifyBeginRecieved(FName NotifyName, const FBranchingPointNotifyPayload& BranchingPointNotifyPayload);

	UFUNCTION()
	void OnNotifyEndRecieved(FName NotifyName, const FBranchingPointNotifyPayload& BranchingPointNotifyPayload);

	UPROPERTY(VisibleAnywhere, ReplicatedUsing = OnRep_CharacterThrowState, Category = "Throw")
	ECharacterThrowState CharacterThrowState = ECharacterThrowState::None;

	UFUNCTION()
	void OnRep_CharacterThrowState(const ECharacterThrowState& OldCharacterThrowState);

	UPROPERTY(EditAnywhere, Category = "Throw", meta = (ClampMin = "0.0", Unit = "ms"))
	float ThrowSpeed = 2000.0f;

	UPROPERTY(EditAnywhere, Category = "Animation")
	UAnimMontage* ThrowMontage = nullptr;

	UPROPERTY(EditAnywhere, Category = "Animation")
	UAnimMontage* CelebrateMontage = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UBoxComponent* InteractionBox;

	void ApplyPowerEffect();
	void EndPowerEffect();

	FOnMontageBlendingOutStarted BlendingOutDelegate;
	FOnMontageEnded MontageEndedDelegate;

	//handle fall out of world
	//Last Position on World when OnGround
	UPROPERTY(replicated)
	FVector LastGroundPosition = FVector::ZeroVector;

	UPROPERTY(ReplicatedUsing = OnRep_IsBeingRescued)
	bool bIsBeingRescued = false;

	UFUNCTION()
	void OnRep_IsBeingRescued();

	//Position From Player when it Hits KillZ
	FVector FallOutOfWorldPosition = FVector::ZeroVector;
	// Used to set a timer from Moving Player back to Ground
	float CurrentRescueTime = 0.0f;

	//Set time that takes to put Player back in Ground
	UPROPERTY(EditAnywhere, Category = "KillZ")
	float TimeToRescuePlayer = 3.f;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	virtual void Landed(const FHitResult& Hit) override;

	virtual void OnMovementModeChanged(EMovementMode PrevMovementMode, uint8 PreviousCustomMode = 0) override;
	virtual void FellOutOfWorld(const class UDamageType& dmgType) override;

	void RequestSprintStart();
	void RequestSprintEnd();

	void RequestJumpStart();
	void RequestJumpStop();

	void Fire();

	void RequestThrowObject();
	void RequestPullorAimObjectStart();
	void RequestPullorAimObjectStop();
	void RequestAim();
	void ResetThrowableObject();

	void RequestUseObject();

	void OnThrowableAttached(AThrowableActor* InThrowableActor);

	void SphereCastPlayerView();
	void SphereCastActorTransform();
	void LineCastActorTransform();
	void ProcessTraceResult(const FHitResult& HitResult, bool bHighlight = true);

	//RPC's actions that can need to be done on the server in order to replicate
	UFUNCTION(Server, Reliable)
	void ServerSprintStart();

	UFUNCTION(Server, Reliable)
	void ServerSprintEnd();

	UFUNCTION(Server, Reliable)
	void ServerPullorAimObject(AThrowableActor* InThrowableActor);

	UFUNCTION(Server, Reliable)
	void ServerRequestPullorAimObject(bool bIsPulling);

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerRequestThrowObject();

	UFUNCTION(Server, Reliable)
	void ServerRequestToggleAim(bool bIsAiming);

	UFUNCTION(NetMulticast, Reliable)
	void MulticastRequestThrowObject();

	UFUNCTION(Client, Reliable)
	void ClientThrowableAttached(AThrowableActor* InThrowableActor);

	UFUNCTION(Server, Reliable)
	void ServerBeginThrow();

	UFUNCTION(Server, Reliable)
	void ServerFinishThrow();

	bool CanThrowObject() const 
	{ 
		return CharacterThrowState == ECharacterThrowState::Attached || CharacterThrowState == ECharacterThrowState::Aiming;
	}

	UFUNCTION(BlueprintPure)
	bool IsPullingObject() const 
	{ 
		return CharacterThrowState == ECharacterThrowState::RequestingPull || CharacterThrowState == ECharacterThrowState::Pulling; 
	}

	UFUNCTION(BlueprintCallable)
	bool AttemptPullObjectAtLocation(const FVector& InLocation);

	UFUNCTION(BlueprintPure)
	bool IsThrowing() const { 
		return CharacterThrowState == ECharacterThrowState::Throwing; 
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
	bool IsBeingRescued() const { 
		return bIsBeingRescued; 
	}

	UFUNCTION(BlueprintPure)
	bool IsHovering() const;

	UFUNCTION(BlueprintPure)
	bool CanAim() const
	{
		return CharacterThrowState == ECharacterThrowState::Attached;
	}

	UFUNCTION(BlueprintPure)
	bool IsAiming() const
	{
		return CharacterThrowState == ECharacterThrowState::Aiming;
	}

	UFUNCTION(BlueprintCallable)
	void NotifyHitByThrowable(AThrowableActor* InThrowable)
	{
		OnStunBegin(1.0f);
	}

	UFUNCTION(Server, Reliable)
	void ServerPlayCelebrateMontage();

	IInteractionInterface* Interface = nullptr;

private:
	UPROPERTY()
	AThrowableActor* ThrowableActor;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	USceneComponent* ProjectileSpawnPoint;

	UPROPERTY(EditDefaultsOnly, Category = "Combat")
	TSubclassOf<class AProjectile> ProjectileClass;

	UFUNCTION()
	void OnBoxBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnBoxEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	void ApplyEffect_Implementation(EEffectType EffectType, bool bIsBuff) override;
	void UpdateEffect(float DeltaTime);
	void EndEffect();

	bool bIsUnderEffect = false;
	bool bIsEffectBuff = false;

	float DefaultEffectCooldown = 5.0f;
	float EffectCooldown = 0.0f;

	EEffectType CurrentEffect = EEffectType::None;

	ATantrumnGameModeBase* GameModeBase;
};
