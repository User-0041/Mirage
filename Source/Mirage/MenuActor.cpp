// Fill out your copyright notice in the Description page of Project Settings.


#include "MenuActor.h"

// Sets default values
AMenuActor::AMenuActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
	//Create Componets
	MenuCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	//Create Menu
	Widget = CreateDefaultSubobject<UWidgetComponent>(TEXT("WidgetComponent"));
}



void AMenuActor::TrastionTo(AMenuActor* Menu, FViewTargetTransitionParams TransitionParams)
{
	APlayerController* PlayerController = UGameplayStatics::GetPlayerController(GetWorld(), 0); 
	PlayerController->SetViewTarget(Menu, TransitionParams);
}

