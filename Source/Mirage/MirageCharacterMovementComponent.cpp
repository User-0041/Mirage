// Fill out your copyright notice in the Description page of Project Settings.

#include "MirageCharacterMovementComponent.h"


#include "MirageCharacter.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/Character.h"
#include "Net/UnrealNetwork.h"


UMirageCharacterMovementComponent::UMirageCharacterMovementComponent()
{
	NavAgentProps.bCanCrouch = true;
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
	Saved_bPrevWantsToCrouch = CharacterMovement->Safe_bPrevWantsToCrouch;
}

void UMirageCharacterMovementComponent::FSavedMove_Mirage::PrepMoveFor(ACharacter* C)
{
	FSavedMove_Character::PrepMoveFor(C);
	UMirageCharacterMovementComponent* CharacterMovement = Cast<UMirageCharacterMovementComponent>(C->GetCharacterMovement());
	CharacterMovement->Safe_bWantsToSprint = Saved_bWantsToSprint;
	CharacterMovement->Safe_bPrevWantsToCrouch= Saved_bPrevWantsToCrouch;
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

	Safe_bPrevWantsToCrouch= bWantsToCrouch;
}

void UMirageCharacterMovementComponent::UpdateCharacterStateBeforeMovement(float DeltaSeconds)
{
	if(MovementMode==MOVE_Walking && !bWantsToCrouch && Safe_bPrevWantsToCrouch)
	{
		
		FHitResult PotentialSlideSurface;
		if(Velocity.SizeSquared() > pow(Slide_MinSpeed,2)&&GetSlideSurface(PotentialSlideSurface) )
		{
			EnterSlide();
		}
	}

	if(IsCustomMovementMode(CMOVE_Slide)&& !bWantsToCrouch)
	{
		ExistSlide();
	}
	Super::UpdateCharacterStateBeforeMovement(DeltaSeconds);
}

void UMirageCharacterMovementComponent::PhysCustom(float deltaTime, int32 Iterations)
{
	Super::PhysCustom(deltaTime, Iterations);
	switch (CustomMovementMode)
	{
	case CMOVE_Slide:
		PhysSlide(deltaTime,Iterations);
		break;
	default:
		UE_LOG(LogTemp,Fatal,TEXT("Are you Dumm???"))		
	}
}

bool UMirageCharacterMovementComponent::IsMovingOnGround() const
{
	return Super::IsMovingOnGround() || IsCustomMovementMode(CMOVE_Slide);
}

bool UMirageCharacterMovementComponent::CanCrouchInCurrentState() const
{
	return Super::CanCrouchInCurrentState() && IsMovingOnGround();
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

void UMirageCharacterMovementComponent::InitializeComponent()
{
	Super::InitializeComponent();
	MirageCharacterOwner = Cast<AMirageCharacter>(GetOwner());
}

void UMirageCharacterMovementComponent::EnterSlide()
{
	UE_LOG(LogTemp, Log, TEXT("Slide Enterd"));
	bWantsToCrouch = true;
	Velocity += Velocity.GetSafeNormal2D()*Slide_EnterImpulse;
	SetMovementMode(MOVE_Custom,CMOVE_Slide);
}

void UMirageCharacterMovementComponent::ExistSlide()
{
	UE_LOG(LogTemp, Log, TEXT("Slide Out"));
	bWantsToCrouch = false;
	FQuat NewRotation = FRotationMatrix::MakeFromXZ(UpdatedComponent->GetForwardVector().GetSafeNormal2D(),FVector::UpVector).ToQuat();
	FHitResult Hit;
	SafeMoveUpdatedComponent(FVector::ZeroVector,NewRotation,true,Hit);
	SetMovementMode(MOVE_Walking);
}

void UMirageCharacterMovementComponent::PhysSlide(float DeltaTime, int32 Iterations)
{
	if(DeltaTime<MIN_TICK_TIME)
	{
		return;
	}
	RestorePreAdditiveRootMotionVelocity();
	
	FHitResult SurfaceHitResult;
	if(!GetSlideSurface(SurfaceHitResult)||Velocity.SizeSquared()<pow(Slide_MinSpeed,2))
	{
		ExistSlide();
		StartNewPhysics(DeltaTime,Iterations);
	}

	Velocity += Slide_GravityForce * FVector::DownVector * DeltaTime;

	if(FMath::Abs(FVector::DotProduct(Acceleration.GetSafeNormal(),UpdatedComponent->GetRightVector() ))>.5)
	{
		Acceleration=Acceleration.ProjectOnTo(UpdatedComponent->GetRightVector());
	}
	else
	{
		Acceleration=FVector::ZeroVector;
	}

	if(!HasAnimRootMotion()&& !CurrentRootMotion.HasOverrideVelocity())
	{
		CalcVelocity(DeltaTime,Slide_Friction,true,GetMaxBrakingDeceleration());
	}
	ApplyRootMotionToVelocity(DeltaTime);

	Iterations ++ ;
	bJustTeleported=false;

	FVector OldLocation = UpdatedComponent->GetComponentLocation();
	FQuat OldRotation = UpdatedComponent->GetComponentRotation().Quaternion();
	FHitResult Hit(1.f);
	FVector Adjusted = Velocity * DeltaTime;
	FVector VelPlanDit = FVector::VectorPlaneProject(Velocity,SurfaceHitResult.Normal).GetSafeNormal();
	FQuat	NewRotation = FRotationMatrix::MakeFromXZ(VelPlanDit,SurfaceHitResult.Normal).ToQuat();
	SafeMoveUpdatedComponent(Adjusted,NewRotation,true,Hit);
	if(Hit.Time<1.f)
	{
		HandleImpact(Hit,DeltaTime,Adjusted);
		SlideAlongSurface(Adjusted,(1.f-Hit.Time),Hit.Normal,Hit,true);
	}

	FHitResult NewSurfaceHitResult;
	if(!GetSlideSurface(NewSurfaceHitResult)||Velocity.SizeSquared()<pow(Slide_MinSpeed,2))
	{
		ExistSlide();
	}

	Velocity = (UpdatedComponent->GetComponentLocation() - OldLocation)/DeltaTime;

}

bool UMirageCharacterMovementComponent::GetSlideSurface(FHitResult& Hit)
{
	FVector Start = UpdatedComponent->GetComponentLocation();
	FVector End = Start + CharacterOwner->GetCapsuleComponent()->GetScaledCapsuleHalfHeight() *2.f * FVector::DownVector;
	FName ProfileName = TEXT("BlockAll");
	return  GetWorld()->LineTraceSingleByProfile(Hit,Start,End,ProfileName,MirageCharacterOwner->GetIgnoreCharacterParams());
}

void UMirageCharacterMovementComponent::SprintPressed()
{
	Safe_bWantsToSprint = true;
}

 void UMirageCharacterMovementComponent::SprintReleased()
{
	 Safe_bWantsToSprint = false;
 }


void UMirageCharacterMovementComponent::CrouchPressed()
 {
	 bWantsToCrouch = !bWantsToCrouch;
 }

bool UMirageCharacterMovementComponent::IsCustomMovementMode(ECustomMovementMode InCustomMovementMovementMode) const
{
	return MovementMode == MOVE_Custom && CustomMovementMode== InCustomMovementMovementMode;
}
