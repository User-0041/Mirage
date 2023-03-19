// Copyright Epic Games, Inc. All Rights Reserved.

#include "MirageGameMode.h"
#include "MirageCharacter.h"
#include "UObject/ConstructorHelpers.h"

AMirageGameMode::AMirageGameMode()
	: Super()
{
	// set default pawn class to our Blueprinted character BAD BAD BAD IDEAL
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnClassFinder(TEXT("/Game/FirstPerson/Blueprints/BP_FirstPersonCharacter"));
	DefaultPawnClass = PlayerPawnClassFinder.Class;

}
