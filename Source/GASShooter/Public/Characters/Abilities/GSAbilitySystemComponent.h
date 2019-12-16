// Copyright 2019 Dan Kestranek.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "GSAbilitySystemComponent.generated.h"

/**
 * 
 */
UCLASS()
class GASSHOOTER_API UGSAbilitySystemComponent : public UAbilitySystemComponent
{
	GENERATED_BODY()
	
public:
	bool CharacterAbilitiesGiven = false;
	bool StartupEffectsApplied = false;
};
