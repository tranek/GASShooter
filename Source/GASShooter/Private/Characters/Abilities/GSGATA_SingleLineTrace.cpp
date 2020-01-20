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

void AGSGATA_SingleLineTrace::StartTargeting(UGameplayAbility* Ability)
{
	SetActorTickEnabled(true);

	// Don't call to Super because we can have more than one Reticle

	OwningAbility = Ability;
	SourceActor = Ability->GetCurrentActorInfo()->AvatarActor.Get();

	/*
	if (ReticleClass)
	{
		AGameplayAbilityWorldReticle* SpawnedReticleActor = GetWorld()->SpawnActor<AGameplayAbilityWorldReticle>(ReticleClass, GetActorLocation(), GetActorRotation());
		if (SpawnedReticleActor)
		{
			SpawnedReticleActor->InitializeReticle(this, MasterPC, ReticleParams);
			ReticleActor = SpawnedReticleActor;

			// This is to catch cases of playing on a listen server where we are using a replicated reticle actor.
			// (In a client controlled player, this would only run on the client and therefor never replicate. If it runs
			// on a listen server, the reticle actor may replicate. We want consistancy between client/listen server players.
			// Just saying 'make the reticle actor non replicated' isnt a good answer, since we want to mix and match reticle
			// actors and there may be other targeting types that want to replicate the same reticle actor class).
			if (!ShouldProduceTargetDataOnServer)
			{
				SpawnedReticleActor->SetReplicates(false);
			}
		}
	}
	*/
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

void AGSGATA_SingleLineTrace::BeginPlay()
{
	Super::BeginPlay();

	// Start with Tick disabled. We'll enable it in StartTargeting() and disable it again in the WaitTargetDataUsingActor task.
	// Ideally we would have a StopTargeting() function on the TargetActor base class.
	// For instant confirmations, tick will never happen because we StartTargeting(), ConfirmTargeting(), and immediately disable tick.
	SetActorTickEnabled(false);
}
