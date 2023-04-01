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
	Saved_bWantsToProne = CharacterMovement->Safe_bWantsToProne;
}

void UMirageCharacterMovementComponent::FSavedMove_Mirage::PrepMoveFor(ACharacter* C)
{
	FSavedMove_Character::PrepMoveFor(C);
	UMirageCharacterMovementComponent* CharacterMovement = Cast<UMirageCharacterMovementComponent>(C->GetCharacterMovement());
	CharacterMovement->Safe_bWantsToSprint = Saved_bWantsToSprint;
	CharacterMovement->Safe_bPrevWantsToCrouch= Saved_bPrevWantsToCrouch;
	CharacterMovement->Safe_bWantsToProne= Saved_bWantsToProne;
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

	if(Safe_bWantsToProne)
	{
		if(CanProne())
		{
			SetMovementMode(MOVE_Custom,CMOVE_Prone);
			if(!CharacterOwner->HasAuthority()) Server_EnterProne();
		}
		Safe_bWantsToProne= false;
	}
	if(IsCustomMovementMode(CMOVE_Prone)&&!bWantsToCrouch)
	{
		SetMovementMode(MOVE_Walking);
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
	case CMOVE_Prone:
		PhysProne(deltaTime,Iterations);
		break;
	default:
		UE_LOG(LogTemp,Fatal,TEXT("Are you Dumm???"))		
	}
}

void UMirageCharacterMovementComponent::OnMovementModeChanged(EMovementMode PreviousMovementMode,
	uint8 PreviousCustomMode)
{
	Super::OnMovementModeChanged(PreviousMovementMode, PreviousCustomMode);
	if(PreviousMovementMode == MOVE_Custom && PreviousCustomMode==CMOVE_Slide){ExistSlide();}
	if(PreviousMovementMode == MOVE_Custom && PreviousCustomMode==CMOVE_Prone){ExistProne();}
		
	if (IsCustomMovementMode(CMOVE_Slide)) EnterSlide();
	if (IsCustomMovementMode(CMOVE_Prone)) EnterProne();

}


bool UMirageCharacterMovementComponent::IsMovingOnGround() const
{
	return Super::IsMovingOnGround() || IsCustomMovementMode(CMOVE_Slide) || IsCustomMovementMode(CMOVE_Prone);
}

bool UMirageCharacterMovementComponent::CanCrouchInCurrentState() const
{
	return Super::CanCrouchInCurrentState() && IsMovingOnGround();
}

float  UMirageCharacterMovementComponent::GetMaxSpeed() const
{
	if(MovementMode!=MOVE_Custom){return Super::GetMaxSpeed();}
	switch (CustomMovementMode)
	{
	case CMOVE_Slide:
		return  Slide_MaxSpeed;
	case CMOVE_Prone:
		return  Prone_MaxSpeed;
	default:
		UE_LOG(LogTemp,Fatal,TEXT("I THINK YOU ARE ACTULLY DUMM"));
		return  -1.f;
	}
}

float UMirageCharacterMovementComponent::GetMaxBrakingDeceleration() const
{
	if(MovementMode!= MOVE_Custom){return Super::GetMaxBrakingDeceleration();}

	switch (CustomMovementMode)
	{
	case CMOVE_Slide:
		return BreakingDecelerationSliding;
	case CMOVE_Prone:
		return BreakingDecelerationProning;
	default:
		UE_LOG(LogTemp,Fatal,TEXT("NAH MAN YOU ARE RELLLLLLLY DUMM LIKE  YOU ARE ACTULLY DUMM"));
		return  -1.f;
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
	bJustTeleported = false;
	bool bCheckedFall=false;
	bool bTriedLedgeMove=false;
	float remainingTime = DeltaTime;
	// Perform the move
	while ( (remainingTime >= MIN_TICK_TIME) && (Iterations < MaxSimulationIterations) && CharacterOwner && (CharacterOwner->Controller || bRunPhysicsWithNoController || (CharacterOwner->GetLocalRole() == ROLE_SimulatedProxy)) )
	{
		Iterations++;
		bJustTeleported = false;
		const float timeTick = GetSimulationTimeStep(remainingTime, Iterations);
		remainingTime -= timeTick;

		// Save current values
		UPrimitiveComponent * const OldBase = GetMovementBase();
		const FVector PreviousBaseLocation = (OldBase != NULL) ? OldBase->GetComponentLocation() : FVector::ZeroVector;
		const FVector OldLocation = UpdatedComponent->GetComponentLocation();
		const FFindFloorResult OldFloor = CurrentFloor;

		// Ensure velocity is horizontal.
		MaintainHorizontalGroundVelocity();
		const FVector OldVelocity = Velocity;

		FVector SlopeForce = CurrentFloor.HitResult.Normal;
		SlopeForce.Z = 0.f;
		Velocity += SlopeForce * Slide_GravityForce * DeltaTime;
		
		Acceleration = Acceleration.ProjectOnTo(UpdatedComponent->GetRightVector().GetSafeNormal2D());

		// Apply acceleration
		CalcVelocity(timeTick, GroundFriction * Slide_Friction, false, GetMaxBrakingDeceleration());
		
		// Compute move parameters
		const FVector MoveVelocity = Velocity;
		const FVector Delta = timeTick * MoveVelocity;
		const bool bZeroDelta = Delta.IsNearlyZero();
		FStepDownResult StepDownResult;
		bool bFloorWalkable = CurrentFloor.IsWalkableFloor();

		if ( bZeroDelta )
		{
			remainingTime = 0.f;
		}
		else
		{
			// try to move forward
			MoveAlongFloor(MoveVelocity, timeTick, &StepDownResult);

			if ( IsFalling() )
			{
				// pawn decided to jump up
				const float DesiredDist = Delta.Size();
				if (DesiredDist > KINDA_SMALL_NUMBER)
				{
					const float ActualDist = (UpdatedComponent->GetComponentLocation() - OldLocation).Size2D();
					remainingTime += timeTick * (1.f - FMath::Min(1.f,ActualDist/DesiredDist));
				}
				StartNewPhysics(remainingTime,Iterations);
				return;
			}
			else if ( IsSwimming() ) //just entered water
			{
				StartSwimming(OldLocation, OldVelocity, timeTick, remainingTime, Iterations);
				return;
			}
		}

		// Update floor.
		// StepUp might have already done it for us.
		if (StepDownResult.bComputedFloor)
		{
			CurrentFloor = StepDownResult.FloorResult;
		}
		else
		{
			FindFloor(UpdatedComponent->GetComponentLocation(), CurrentFloor, bZeroDelta, NULL);
		}


		// check for ledges here
		const bool bCheckLedges = !CanWalkOffLedges();
		if ( bCheckLedges && !CurrentFloor.IsWalkableFloor() )
		{
			// calculate possible alternate movement
			const FVector GravDir = FVector(0.f,0.f,-1.f);
			const FVector NewDelta = bTriedLedgeMove ? FVector::ZeroVector : GetLedgeMove(OldLocation, Delta, GravDir);
			if ( !NewDelta.IsZero() )
			{
				// first revert this move
				RevertMove(OldLocation, OldBase, PreviousBaseLocation, OldFloor, false);

				// avoid repeated ledge moves if the first one fails
				bTriedLedgeMove = true;

				// Try new movement direction
				Velocity = NewDelta / timeTick;
				remainingTime += timeTick;
				continue;
			}
			else
			{
				// see if it is OK to jump
				// @todo collision : only thing that can be problem is that oldbase has world collision on
				bool bMustJump = bZeroDelta || (OldBase == NULL || (!OldBase->IsQueryCollisionEnabled() && MovementBaseUtility::IsDynamicBase(OldBase)));
				if ( (bMustJump || !bCheckedFall) && CheckFall(OldFloor, CurrentFloor.HitResult, Delta, OldLocation, remainingTime, timeTick, Iterations, bMustJump) )
				{
					return;
				}
				bCheckedFall = true;

				// revert this move
				RevertMove(OldLocation, OldBase, PreviousBaseLocation, OldFloor, true);
				remainingTime = 0.f;
				break;
			}
		}
		else
		{
			// Validate the floor check
			if (CurrentFloor.IsWalkableFloor())
			{
				if (ShouldCatchAir(OldFloor, CurrentFloor))
				{
					HandleWalkingOffLedge(OldFloor.HitResult.ImpactNormal, OldFloor.HitResult.Normal, OldLocation, timeTick);
					if (IsMovingOnGround())
					{
						// If still walking, then fall. If not, assume the user set a different mode they want to keep.
						StartFalling(Iterations, remainingTime, timeTick, Delta, OldLocation);
					}
					return;
				}

				AdjustFloorHeight();
				SetBase(CurrentFloor.HitResult.Component.Get(), CurrentFloor.HitResult.BoneName);
			}
			else if (CurrentFloor.HitResult.bStartPenetrating && remainingTime <= 0.f)
			{
				// The floor check failed because it started in penetration
				// We do not want to try to move downward because the downward sweep failed, rather we'd like to try to pop out of the floor.
				FHitResult Hit(CurrentFloor.HitResult);
				Hit.TraceEnd = Hit.TraceStart + FVector(0.f, 0.f, MAX_FLOOR_DIST);
				const FVector RequestedAdjustment = GetPenetrationAdjustment(Hit);
				ResolvePenetration(RequestedAdjustment, Hit, UpdatedComponent->GetComponentQuat());
				bForceNextFloorCheck = true;
			}

			// check if just entered water
			if ( IsSwimming() )
			{
				StartSwimming(OldLocation, Velocity, timeTick, remainingTime, Iterations);
				return;
			}

			// See if we need to start falling.
			if (!CurrentFloor.IsWalkableFloor() && !CurrentFloor.HitResult.bStartPenetrating)
			{
				const bool bMustJump = bJustTeleported || bZeroDelta || (OldBase == NULL || (!OldBase->IsQueryCollisionEnabled() && MovementBaseUtility::IsDynamicBase(OldBase)));
				if ((bMustJump || !bCheckedFall) && CheckFall(OldFloor, CurrentFloor.HitResult, Delta, OldLocation, remainingTime, timeTick, Iterations, bMustJump) )
				{
					return;
				}
				bCheckedFall = true;
			}
		}
		
		// Allow overlap events and such to change physics state and velocity
		if (IsMovingOnGround() && bFloorWalkable)
		{
			// Make velocity reflect actual move
			if( !bJustTeleported && !HasAnimRootMotion() && !CurrentRootMotion.HasOverrideVelocity() && timeTick >= MIN_TICK_TIME)
			{
				// TODO-RootMotionSource: Allow this to happen during partial override Velocity, but only set allowed axes?
				Velocity = (UpdatedComponent->GetComponentLocation() - OldLocation) / timeTick;
				MaintainHorizontalGroundVelocity();
			}
		}

		// If we didn't move at all this iteration then abort (since future iterations will also be stuck).
		if (UpdatedComponent->GetComponentLocation() == OldLocation)
		{
			remainingTime = 0.f;
			break;
		}
	}


	FHitResult Hit;
	FQuat NewRotation = FRotationMatrix::MakeFromXZ(Velocity.GetSafeNormal2D(), FVector::UpVector).ToQuat();
	SafeMoveUpdatedComponent(FVector::ZeroVector, NewRotation, false, Hit);
}

bool UMirageCharacterMovementComponent::CanSlide() const
{
	FVector Start = UpdatedComponent->GetComponentLocation();
	FVector End = Start + CharacterOwner->GetCapsuleComponent()->GetScaledCapsuleHalfHeight() * 50.f * FVector::DownVector;
	FName ProfileName= TEXT("BlockAll");
	bool ValidSurface = GetWorld()->LineTraceTestByProfile(Start,End,ProfileName,MirageCharacterOwner->GetIgnoreCharacterParams());
	bool bEnoughSpeed =  Velocity.SizeSquared() > pow(Slide_MinSpeed ,2);
	return ValidSurface && bEnoughSpeed ;
}

bool UMirageCharacterMovementComponent::GetSlideSurface(FHitResult& Hit)
{
	FVector Start = UpdatedComponent->GetComponentLocation();
	FVector End = Start + CharacterOwner->GetCapsuleComponent()->GetScaledCapsuleHalfHeight() *50.f * FVector::DownVector;
	FName ProfileName = TEXT("BlockAll");
	return  GetWorld()->LineTraceSingleByProfile(Hit,Start,End,ProfileName,MirageCharacterOwner->GetIgnoreCharacterParams());
}

void UMirageCharacterMovementComponent::EnterProne()
{
	bWantsToCrouch= true;
	FindFloor(UpdatedComponent->GetComponentLocation(),CurrentFloor,true,nullptr);
}

void UMirageCharacterMovementComponent::ExistProne()
{
}

bool UMirageCharacterMovementComponent::CanProne() const
{
	return  IsCustomMovementMode(CMOVE_Slide)|| IsMovementMode(MOVE_Walking) && IsCrouching();
}

void UMirageCharacterMovementComponent::PhysProne(float deltaTime, int32 Iterations)
{

	if(deltaTime<MIN_TICK_TIME)
	{
		return;
	}
	if (!CharacterOwner || (!CharacterOwner->Controller && !bRunPhysicsWithNoController && !HasAnimRootMotion() && !CurrentRootMotion.HasOverrideVelocity() && (CharacterOwner->GetLocalRole() != ROLE_SimulatedProxy)))
	{
		Acceleration = FVector::ZeroVector;
		Velocity = FVector::ZeroVector;
		return;
	}

	bJustTeleported=false;
	bool bCheckedFall= false;
	bool bTriedLedgeMove = false;
	float remainingTime = deltaTime;
	while ((remainingTime >= MIN_TICK_TIME) && (Iterations < MaxSimulationIterations) && CharacterOwner && (CharacterOwner->Controller || bRunPhysicsWithNoController || (CharacterOwner->GetLocalRole() == ROLE_SimulatedProxy)))
	{
		Iterations++;
		bJustTeleported= false;
		const float timeTick= GetSimulationTimeStep(remainingTime,Iterations);
		remainingTime-= timeTick;

		//Save Current Values
		UPrimitiveComponent * const OldBase = GetMovementBase();
		const FVector PreviousBaseLocation = (OldBase != nullptr) ? OldBase->GetComponentLocation() : FVector::ZeroVector;
		const FVector OldLocation = UpdatedComponent->GetComponentLocation();
		const FFindFloorResult OldFloor = CurrentFloor;
		//Velocity is Horizontal
		MaintainHorizontalGroundVelocity();
		const FVector OldVelocity=Velocity;
		Acceleration.Z= 0.f;
		//Apply Acceleration
		CalcVelocity(timeTick,GroundFriction,false,GetMaxBrakingDeceleration());
		//Compute Move Parameters
		const FVector MoveVelocity = Velocity;
		const FVector Delta =  timeTick * Velocity;
		const bool bZeroDelta= Delta.IsNearlyZero();
		FStepDownResult StepDownResult;

		if(bZeroDelta)
		{
			remainingTime=0.f;
		}else
		{
			MoveAlongFloor(MoveVelocity,timeTick,&StepDownResult);
			if(IsFalling())
			{
				const float DesiredDist= Delta.Size();
				if(DesiredDist>KINDA_SMALL_NUMBER)
				{
				
					const float ActualDist = (UpdatedComponent->GetComponentLocation() - OldLocation).Size2D();
					remainingTime += timeTick * (1.f - FMath::Min(1.f,ActualDist/DesiredDist));
				}
				StartNewPhysics(remainingTime,Iterations);
				return;
			}
			else if (IsSwimming())
			{
				StartSwimming(OldLocation,OldVelocity,timeTick,remainingTime,Iterations);
				return;
			}
		}
		
		//Update floor
		if(StepDownResult.bComputedFloor)
		{
		CurrentFloor=StepDownResult.FloorResult;	
		}
		else
		{
			FindFloor(UpdatedComponent->GetComponentLocation(),CurrentFloor,bZeroDelta,nullptr);
		}
			// check for ledges here
		const bool bCheckLedges = !CanWalkOffLedges();
		if ( bCheckLedges && !CurrentFloor.IsWalkableFloor() )
		{
			// calculate possible alternate movement
			const FVector GravDir = FVector(0.f,0.f,-1.f);
			const FVector NewDelta = bTriedLedgeMove ? FVector::ZeroVector : GetLedgeMove(OldLocation, Delta, GravDir);
			if ( !NewDelta.IsZero() )
			{
				// first revert this move
				RevertMove(OldLocation, OldBase, PreviousBaseLocation, OldFloor, false);

				// avoid repeated ledge moves if the first one fails
				bTriedLedgeMove = true;

				// Try new movement direction
				Velocity = NewDelta/timeTick; // v = dx/dt
				remainingTime += timeTick;
				continue;
			}
			else
			{
				// see if it is OK to jump
				// @todo collision : only thing that can be problem is that oldbase has world collision on
				bool bMustJump = bZeroDelta || (OldBase == NULL || (!OldBase->IsQueryCollisionEnabled() && MovementBaseUtility::IsDynamicBase(OldBase)));
				if ( (bMustJump || !bCheckedFall) && CheckFall(OldFloor, CurrentFloor.HitResult, Delta, OldLocation, remainingTime, timeTick, Iterations, bMustJump) )
				{
					return;
				}
				bCheckedFall = true;

				// revert this move
				RevertMove(OldLocation, OldBase, PreviousBaseLocation, OldFloor, true);
				remainingTime = 0.f;
				break;
			}
		}
		else
		{
			// Validate the floor check
			if (CurrentFloor.IsWalkableFloor())
			{
				AdjustFloorHeight();
				SetBase(CurrentFloor.HitResult.Component.Get(), CurrentFloor.HitResult.BoneName);
			}
			else if (CurrentFloor.HitResult.bStartPenetrating && remainingTime <= 0.f)
			{
				// The floor check failed because it started in penetration
				// We do not want to try to move downward because the downward sweep failed, rather we'd like to try to pop out of the floor.
				FHitResult Hit(CurrentFloor.HitResult);
				Hit.TraceEnd = Hit.TraceStart + FVector(0.f, 0.f, MAX_FLOOR_DIST);
				const FVector RequestedAdjustment = GetPenetrationAdjustment(Hit);
				ResolvePenetration(RequestedAdjustment, Hit, UpdatedComponent->GetComponentQuat());
				bForceNextFloorCheck = true;
			}

			// check if just entered water
			if ( IsSwimming() )
			{
				StartSwimming(OldLocation, Velocity, timeTick, remainingTime, Iterations);
				return;
			}

			// See if we need to start falling.
			if (!CurrentFloor.IsWalkableFloor() && !CurrentFloor.HitResult.bStartPenetrating)
			{
				const bool bMustJump = bJustTeleported || bZeroDelta || (OldBase == NULL || (!OldBase->IsQueryCollisionEnabled() && MovementBaseUtility::IsDynamicBase(OldBase)));
				if ((bMustJump || !bCheckedFall) && CheckFall(OldFloor, CurrentFloor.HitResult, Delta, OldLocation, remainingTime, timeTick, Iterations, bMustJump) )
				{
					return;
				}
				bCheckedFall = true;
			}
		}
		
		// Allow overlap events and such to change physics state and velocity
		if (IsMovingOnGround())
		{
			// Make velocity reflect actual move
			if( !bJustTeleported && !HasAnimRootMotion() && !CurrentRootMotion.HasOverrideVelocity() && timeTick >= MIN_TICK_TIME)
			{
				// TODO-RootMotionSource: Allow this to happen during partial override Velocity, but only set allowed axes?
				Velocity = (UpdatedComponent->GetComponentLocation() - OldLocation) / timeTick; // v = dx / dt
				MaintainHorizontalGroundVelocity();
			}
		}

		// If we didn't move at all this iteration then abort (since future iterations will also be stuck).
		if (UpdatedComponent->GetComponentLocation() == OldLocation)
		{
			remainingTime = 0.f;
			break;
		}
	}

	if (IsMovingOnGround()) 
	{
		MaintainHorizontalGroundVelocity();
	};
	
	}


void UMirageCharacterMovementComponent::Server_EnterProne_Implementation()
{
	Safe_bWantsToProne= true;
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
	GetWorld()->GetTimerManager().SetTimer(TimerHandle_EnterProne,this,&UMirageCharacterMovementComponent::TryEnterProne,Prone_EnterHoldDuration);
 }

void UMirageCharacterMovementComponent::CrouchReleased()
{
	GetWorld()->GetTimerManager().ClearTimer(TimerHandle_EnterProne);
}

bool UMirageCharacterMovementComponent::IsCustomMovementMode(ECustomMovementMode InCustomMovementMovementMode) const
{
	return MovementMode == MOVE_Custom && CustomMovementMode== InCustomMovementMovementMode;
}

bool UMirageCharacterMovementComponent::IsMovementMode(EMovementMode InMovementMode) const
{
	return  InMovementMode==MovementMode;
}