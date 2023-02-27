// Fill out your copyright notice in the Description page of Project Settings.


#include "MenuWidget.h"

void UMenuWidget::CallTransitionToLinkedMenu(int index)
{
	TransitionToLinkedMenu.Broadcast(index);
}
