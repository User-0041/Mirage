// Fill out your copyright notice in the Description page of Project Settings.

#pragma once


#include "CoreMinimal.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "MirageCharacterMovementComponent.generated.h"
/**
 * 
 */

class AMirageCharacter;
UENUM(BlueprintType)
enum ECustomMovementMode
{
	CMOVE_None UMETA(Hidden),
	CMOVE_Slide UMETA(DisplayName= "Slide"),
	CMOVE_Max UMETA(Hidden)
};

UCLASS()
class MIRAGE_API UMirageCharacterMovementComponent : public UCharacterMovementComponent
{
	GENERATED_BODY()

	class FSavedMove_Mirage : public FSavedMove_Character
	{
		//Flags
		uint8 Saved_bWantsToSprint : 1;

		uint8 Saved_bPrevWantsToCrouch : 1;

		
		virtual bool
		CanCombineWith(const FSavedMovePtr& NewMove, ACharacter* InCharacter, float MaxDelta) const override;
		virtual void Clear() override;
		virtual uint8 GetCompressedFlags() const override;
		virtual void SetMoveFor(ACharacter* C, float InDeltaTime, FVector const& NewAccel,FNetworkPredictionData_Client_Character& ClientData) override;
		virtual void PrepMoveFor(ACharacter* C) override;
	};


	class FNetworkPredictionData_Client_Mirage : public FNetworkPredictionData_Client_Character
	{
	public:
		FNetworkPredictionData_Client_Mirage(const UCharacterMovementComponent& ClientMovement);
		virtual FSavedMovePtr AllocateNewMove() override;
	};

	UPROPERTY(Transient) AMirageCharacter* MirageCharacterOwner;

protected:
	virtual void UpdateFromCompressedFlags(uint8 Flags) override;
	virtual void OnMovementUpdated(float DeltaSeconds, const FVector& OldLocation, const FVector& OldVelocity) override;
	virtual void UpdateCharacterStateBeforeMovement(float DeltaSeconds) override;
	virtual void PhysCustom(float deltaTime, int32 Iterations) override;

public:
	virtual bool IsMovingOnGround() const override;
	virtual bool CanCrouchInCurrentState() const override;
	bool Safe_bWantsToSprint;
	bool Safe_bPrevWantsToCrouch;

public:
	UMirageCharacterMovementComponent();
	virtual FNetworkPredictionData_Client* GetPredictionData_Client() const override;
	virtual void InitializeComponent() override;

private:
	void EnterSlide();
	void ExistSlide();
	void PhysSlide(float DeltaTime , int32 Iterations);
	bool GetSlideSurface(FHitResult& Hit);
public:
	UPROPERTY(EditDefaultsOnly)
	float MaxSprintSpeed = 750.f;
	UPROPERTY(EditDefaultsOnly)
	float WalkSprintSpeed = 50.;
	
	UPROPERTY(EditDefaultsOnly)
	float Slide_MinSpeed= 0;
	UPROPERTY(EditDefaultsOnly)
	float Slide_EnterImpulse= 500;
	UPROPERTY(EditDefaultsOnly)
	float Slide_GravityForce= 5000;
	UPROPERTY(EditDefaultsOnly)
	float Slide_Friction= 0.2f;
	


	UFUNCTION(BlueprintCallable)
	void SprintPressed();
	UFUNCTION(BlueprintCallable)
	void SprintReleased();
	UFUNCTION(BlueprintCallable)
	void CrouchPressed();
	UFUNCTION(BlueprintCallable)
	bool IsCustomMovementMode(ECustomMovementMode InCustomMovementMovementMode) const;
	
};
