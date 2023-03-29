// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Camera/PlayerCameraManager.h"

#include  "PLayerCharacterCameraManager.generated.h"

/**
 * 
 */
UCLASS()
 class MIRAGE_API APLayerCharacterCameraManager : public APlayerCameraManager
{
	GENERATED_BODY()
	UPROPERTY(EditDefaultsOnly) float CrouchBlendDuration = 0.5f;
	float CrouchBlendTime;
public:
	APLayerCharacterCameraManager();
	virtual void UpdateViewTarget(FTViewTarget& OutVT, float DeltaTime)override;
};
