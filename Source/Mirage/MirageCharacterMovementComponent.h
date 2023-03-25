// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Character.h"
#include "MirageCharacterMovementComponent.generated.h"
/**
 * 
 */



UCLASS()
class MIRAGE_API UMirageCharacterMovementComponent : public UCharacterMovementComponent
{
	GENERATED_BODY()

	class FSavedMove_Mirage : public FSavedMove_Character {
		uint8 Saved_bWantsToSprint : 1;
		virtual bool CanCombineWith(const FSavedMovePtr& NewMove, ACharacter* InCharacter, float MaxDelta) const override;
		virtual void Clear() override;
		virtual uint8 GetCompressedFlags() const override;
		virtual void SetMoveFor(ACharacter* C, float InDeltaTime, FVector const& NewAccel, FNetworkPredictionData_Client_Character& ClientData) override;
		virtual void PrepMoveFor(ACharacter* C) override;
	};


	class FNetworkPredictionData_Client_Mirage : public FNetworkPredictionData_Client_Character {
	public:
		FNetworkPredictionData_Client_Mirage(const UCharacterMovementComponent& ClientMovement);
		virtual FSavedMovePtr AllocateNewMove() override;
	};
protected:
	virtual void UpdateFromCompressedFlags(uint8 Flags)  override;
	virtual void OnMovementUpdated(float DeltaSeconds, const FVector& OldLocation, const FVector& OldVelocity);

public:
	UMirageCharacterMovementComponent();
	virtual FNetworkPredictionData_Client* GetPredictionData_Client() const override;
	bool Safe_bWantsToSprint;

public:
	UPROPERTY(EditDefaultsOnly) float MaxSprintSpeed = 750.f;
	UPROPERTY(EditDefaultsOnly) float WalkSprintSpeed = 50.;
	UFUNCTION(BlueprintCallable) void SprintPressed();
	UFUNCTION(BlueprintCallable) void SprintReleased();
};
