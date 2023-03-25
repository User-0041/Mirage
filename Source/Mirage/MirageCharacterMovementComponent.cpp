// Fill out your copyright notice in the Description page of Project Settings.


#include "MirageCharacterMovementComponent.h"

#include"Mirage.h"
#include "GameFramework/Character.h"

UMirageCharacterMovementComponent::UMirageCharacterMovementComponent()
{
}


bool UMirageCharacterMovementComponent::FSavedMove_Mirage::CanCombineWith(const FSavedMovePtr& NewMove, ACharacter* InCharacter, float MaxDelta) const
{
	FSavedMove_Mirage* NewMirageMove = static_cast<FSavedMove_Mirage*>(NewMove.Get());
	if (Saved_bWantsToSprint!=NewMirageMove->Saved_bWantsToSprint) {
		return false;
	}
	return FSavedMove_Character::CanCombineWith(NewMove,InCharacter,MaxDelta);
}

void UMirageCharacterMovementComponent::FSavedMove_Mirage::Clear()
{
	FSavedMove_Character::Clear();
	Saved_bWantsToSprint = 0;
}

uint8 UMirageCharacterMovementComponent::FSavedMove_Mirage::GetCompressedFlags() const
{
	uint8 Result = FSavedMove_Character::GetCompressedFlags();
	if (Saved_bWantsToSprint) Result |= FLAG_Custom_0;
	return Result;
}

void UMirageCharacterMovementComponent::FSavedMove_Mirage::SetMoveFor(ACharacter* C, float InDeltaTime, FVector const& NewAccel, FNetworkPredictionData_Client_Character& ClientData)
{
	FSavedMove_Character::SetMoveFor(C, InDeltaTime, NewAccel, ClientData);
	UMirageCharacterMovementComponent* CharacterMovement = Cast<UMirageCharacterMovementComponent>(C->GetCharacterMovement());
	Saved_bWantsToSprint = CharacterMovement->Safe_bWantsToSprint;
}

void UMirageCharacterMovementComponent::FSavedMove_Mirage::PrepMoveFor(ACharacter* C)
{
	FSavedMove_Character::PrepMoveFor(C);
	UMirageCharacterMovementComponent* CharacterMovement = Cast<UMirageCharacterMovementComponent>(C->GetCharacterMovement());
	CharacterMovement->Safe_bWantsToSprint = Saved_bWantsToSprint;
}

UMirageCharacterMovementComponent::FNetworkPredictionData_Client_Mirage::FNetworkPredictionData_Client_Mirage(const UCharacterMovementComponent& ClientMovement) : FNetworkPredictionData_Client_Character(ClientMovement)
{
}

FSavedMovePtr UMirageCharacterMovementComponent::FNetworkPredictionData_Client_Mirage::AllocateNewMove()
{
	return FSavedMovePtr(new FSavedMove_Mirage());
}

void UMirageCharacterMovementComponent::UpdateFromCompressedFlags(uint8 Flags) 
{
	Super::UpdateFromCompressedFlags(Flags);
	Safe_bWantsToSprint = (Flags & FSavedMove_Character::FLAG_Custom_0) != 0;
}

void UMirageCharacterMovementComponent::OnMovementUpdated(float DeltaSeconds, const FVector& OldLocation, const FVector& OldVelocity)
{
	Super::OnMovementUpdated(DeltaSeconds, OldLocation, OldVelocity);
	if (MovementMode == MOVE_Walking) {
		if (Safe_bWantsToSprint) {
			MaxWalkSpeed = MaxSprintSpeed;
		}
		else if (!Safe_bWantsToSprint) {
			MaxWalkSpeed = WalkSprintSpeed;
		}
	}
}


FNetworkPredictionData_Client* UMirageCharacterMovementComponent::GetPredictionData_Client() const
{

	check(PawnOwner != nullptr)

		if (ClientPredictionData == nullptr)
		{
			UMirageCharacterMovementComponent* MutableThis = const_cast<UMirageCharacterMovementComponent*>(this);
			MutableThis->ClientPredictionData = new FNetworkPredictionData_Client_Mirage(*this);
			MutableThis->ClientPredictionData->MaxSmoothNetUpdateDist = 92.f;
			MutableThis->ClientPredictionData->NoSmoothNetUpdateDist = 140.f;
		}
	return ClientPredictionData;
}

void UMirageCharacterMovementComponent::SprintPressed()
{
	Safe_bWantsToSprint = true;
}

 void UMirageCharacterMovementComponent::SprintReleased()
{
	 Safe_bWantsToSprint = false;
 }
