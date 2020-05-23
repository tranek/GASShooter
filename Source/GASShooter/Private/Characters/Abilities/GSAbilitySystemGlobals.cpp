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

void UGSAbilitySystemGlobals::InitGlobalTags()
{
	Super::InitGlobalTags();

	DeadTag = FGameplayTag::RequestGameplayTag("State.Dead");
	KnockedDownTag = FGameplayTag::RequestGameplayTag("State.KnockedDown");
	InteractingTag = FGameplayTag::RequestGameplayTag("State.Interacting");
	InteractingRemovalTag = FGameplayTag::RequestGameplayTag("State.InteractingRemoval");
}
