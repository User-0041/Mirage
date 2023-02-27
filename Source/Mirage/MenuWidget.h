// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MenuWidget.generated.h"

/**
 * 
 */
UCLASS()
class MIRAGE_API UMenuWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	/** Please add a variable description */
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FTransitionToLinkedMenu , int32, Index);
	UPROPERTY(BlueprintAssignable, EditDefaultsOnly, Category = "Default")
	FTransitionToLinkedMenu TransitionToLinkedMenu;


	UFUNCTION(BlueprintCallable)
		void CallTransitionToLinkedMenu(int Index);

};
