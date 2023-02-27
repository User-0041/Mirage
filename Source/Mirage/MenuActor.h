// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/GameplayStatics.h" 
#include "GameFramework/Actor.h"
#include "Camera/CameraComponent.h" 
#include "Components/WidgetComponent.h" 
#include "MenuActor.generated.h"


UCLASS()
class MIRAGE_API AMenuActor : public AActor
{
	GENERATED_BODY()


public:
	// Sets default values for this actor's properties
	AMenuActor();


	/** First person camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
		class UCameraComponent* MenuCameraComponent;

	/** Please add a variable description */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
		class UWidgetComponent* Widget;
	/** Please add a variable description */
	UPROPERTY(BlueprintReadOnly,EditInstanceOnly, Category = "Default")
		TArray<AMenuActor*> LinkedMenus;
public:
	//TrastionFunction
	UFUNCTION(BlueprintCallable)
		void TrastionTo(AMenuActor* Menu, FViewTargetTransitionParams TransitionParams);
};
