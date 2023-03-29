// Fill out your copyright notice in the Description page of Project Settings.


#include "PLayerCharacterCameraManager.h"
#include"MirageCharacterMovementComponent.h"
#include"MirageCharacter.h"
#include "Components/CapsuleComponent.h"
APLayerCharacterCameraManager::APLayerCharacterCameraManager()
{
}

void APLayerCharacterCameraManager::UpdateViewTarget(FTViewTarget& OutVT, float DeltaTime)
{
	Super::UpdateViewTarget(OutVT, DeltaTime);
	if (AMirageCharacter* MirageCharacter = Cast<AMirageCharacter>(GetOwningPlayerController()->GetPawn())) {
		
		UMirageCharacterMovementComponent* MirageCharacterMovementComponent = MirageCharacter->CharacterMirageMovementComponentGet();
		FVector TargetCrouchOffSet = FVector(
			0,
			0,
			MirageCharacterMovementComponent->GetCrouchedHalfHeight() - MirageCharacter->GetClass()->GetDefaultObject<ACharacter>()->GetCapsuleComponent()->GetScaledCapsuleHalfHeight()
		);

		FVector OffSet = FMath::Lerp(FVector::ZeroVector, TargetCrouchOffSet, FMath::Clamp(CrouchBlendTime / CrouchBlendDuration, 0.f, 1.f));

		if (MirageCharacterMovementComponent->IsCrouching()) {
			CrouchBlendTime = FMath::Clamp(CrouchBlendTime + DeltaTime, 0.f, CrouchBlendDuration);
			OffSet -= TargetCrouchOffSet;
		}
		else
		{
			CrouchBlendTime = FMath::Clamp(CrouchBlendTime - DeltaTime, 0.f, CrouchBlendDuration);
		}
		if (MirageCharacterMovementComponent->IsMovingOnGround()) {
			OutVT.POV.Location += OffSet;
		}

	}
}
