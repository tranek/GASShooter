// Copyright 2020 Dan Kestranek.


#include "Characters/Abilities/GSGATA_SingleLineTrace.h"
#include "AbilitySystemComponent.h"
#include "GameFramework/PlayerController.h"
#include "GameplayAbilitySpec.h"

AGSGATA_SingleLineTrace::AGSGATA_SingleLineTrace()
{
	bDestroyOnConfirmation = false;
}

void AGSGATA_SingleLineTrace::SetStartLocation(const FGameplayAbilityTargetingLocationInfo& InStartLocation)
{
	StartLocation = InStartLocation;
}

void AGSGATA_SingleLineTrace::SetShouldProduceTargetDataOnServer(bool InShouldProduceTargetDataOnServer)
{
	ShouldProduceTargetDataOnServer = InShouldProduceTargetDataOnServer;
}

void AGSGATA_SingleLineTrace::SetDestroyOnConfirmation(bool DestroyOnConfirmation)
{
	bDestroyOnConfirmation = DestroyOnConfirmation;
}

void AGSGATA_SingleLineTrace::CancelTargeting()
{
	const FGameplayAbilityActorInfo* ActorInfo = (OwningAbility ? OwningAbility->GetCurrentActorInfo() : nullptr);
	UAbilitySystemComponent* ASC = (ActorInfo ? ActorInfo->AbilitySystemComponent.Get() : nullptr);
	if (ASC)
	{
		ASC->AbilityReplicatedEventDelegate(EAbilityGenericReplicatedEvent::GenericCancel, OwningAbility->GetCurrentAbilitySpecHandle(), OwningAbility->GetCurrentActivationInfo().GetActivationPredictionKey()).Remove(GenericCancelHandle);
	}
	else
	{
		ABILITY_LOG(Warning, TEXT("AGameplayAbilityTargetActor::CancelTargeting called with null ASC! Actor %s"), *GetName());
	}

	CanceledDelegate.Broadcast(FGameplayAbilityTargetDataHandle());
}
