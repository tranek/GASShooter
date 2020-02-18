// Copyright 2020 Dan Kestranek.


#include "Characters/Abilities/GSAbilitySystemGlobals.h"
#include "Characters/Abilities/GSGameplayEffectTypes.h"

UGSAbilitySystemGlobals::UGSAbilitySystemGlobals()
{

}

FGameplayEffectContext* UGSAbilitySystemGlobals::AllocGameplayEffectContext() const
{
	return new FGSGameplayEffectContext();
}
