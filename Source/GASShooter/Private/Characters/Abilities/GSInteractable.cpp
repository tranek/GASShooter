// Copyright 2020 Dan Kestranek.


#include "Characters/Abilities/GSInteractable.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"

bool IGSInteractable::IsAvailableForInteraction_Implementation(UPrimitiveComponent* InteractionComponent) const
{
	return false;
}

float IGSInteractable::GetInteractionDuration_Implementation(UPrimitiveComponent* InteractionComponent) const
{
	return 0.0f;
}

void IGSInteractable::GetPreInteractSyncType_Implementation(bool& bShouldSync, EAbilityTaskNetSyncType& Type, UPrimitiveComponent* InteractionComponent) const
{
	bShouldSync = false;
	Type = EAbilityTaskNetSyncType::OnlyServerWait;
}

void IGSInteractable::GetPostInteractSyncType_Implementation(bool& bShouldSync, EAbilityTaskNetSyncType& Type, UPrimitiveComponent* InteractionComponent) const
{
	bShouldSync = false;
	Type = EAbilityTaskNetSyncType::OnlyServerWait;
}

FSimpleMulticastDelegate* IGSInteractable::GetTargetCancelInteractionDelegate(UPrimitiveComponent* InteractionComponent)
{
	return nullptr;
}

void IGSInteractable::RegisterInteracter_Implementation(UPrimitiveComponent* InteractionComponent, AActor* InteractingActor)
{
	if (Interacters.Contains(InteractionComponent))
	{
		TArray<AActor*>& InteractingActors = Interacters[InteractionComponent];
		if (!InteractingActors.Contains(InteractingActor))
		{
			InteractingActors.Add(InteractingActor);
		}
	}
	else
	{
		TArray<AActor*> InteractingActors;
		InteractingActors.Add(InteractingActor);
		Interacters.Add(InteractionComponent, InteractingActors);
	}
}

void IGSInteractable::UnregisterInteracter_Implementation(UPrimitiveComponent* InteractionComponent, AActor* InteractingActor)
{
	if (Interacters.Contains(InteractionComponent))
	{
		TArray<AActor*>& InteractingActors = Interacters[InteractionComponent];
		InteractingActors.Remove(InteractingActor);
	}
}

void IGSInteractable::InteractableCancelInteraction_Implementation(UPrimitiveComponent* InteractionComponent)
{
	if (Interacters.Contains(InteractionComponent))
	{
		FGameplayTagContainer InteractAbilityTagContainer;
		InteractAbilityTagContainer.AddTag(FGameplayTag::RequestGameplayTag("Ability.Interaction"));

		TArray<AActor*>& InteractingActors = Interacters[InteractionComponent];
		for (AActor* InteractingActor : InteractingActors)
		{
			UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(InteractingActor);

			if (ASC)
			{
				ASC->CancelAbilities(&InteractAbilityTagContainer);
			}
		}

		InteractingActors.Empty();
	}
}
