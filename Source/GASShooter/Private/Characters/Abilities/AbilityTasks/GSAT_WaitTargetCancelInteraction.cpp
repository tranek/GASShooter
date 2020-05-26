// Copyright 2020 Dan Kestranek.


#include "Characters/Abilities/AbilityTasks/GSAT_WaitTargetCancelInteraction.h"
#include "Characters/Abilities/GSInteractable.h"

UGSAT_WaitTargetCancelInteraction::UGSAT_WaitTargetCancelInteraction(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

UGSAT_WaitTargetCancelInteraction* UGSAT_WaitTargetCancelInteraction::WaitForTargetCancelInteraction(UGameplayAbility* OwningAbility, UPrimitiveComponent* InteractingComponent)
{
	UGSAT_WaitTargetCancelInteraction* MyObj = NewAbilityTask<UGSAT_WaitTargetCancelInteraction>(OwningAbility);
	MyObj->InteractingComponent = InteractingComponent;
	return MyObj;
}

void UGSAT_WaitTargetCancelInteraction::Activate()
{
	if (!InteractingComponent.IsValid())
	{
		return;
	}

	IGSInteractable* Interactable = Cast<IGSInteractable>(InteractingComponent->GetOwner());
	if (!Interactable)
	{
		return;
	}

	InteractionCanceledDelegateHandle = Interactable->GetTargetCancelInteractionDelegate(InteractingComponent.Get())->AddUObject(this, &UGSAT_WaitTargetCancelInteraction::OnInteractionCanceled);
}

void UGSAT_WaitTargetCancelInteraction::OnInteractionCanceled()
{
	InteractionCanceled.Broadcast();
	EndTask();
}

void UGSAT_WaitTargetCancelInteraction::OnDestroy(bool AbilityEnded)
{
	if (InteractingComponent.IsValid())
	{
		IGSInteractable* Interactable = Cast<IGSInteractable>(InteractingComponent->GetOwner());
		if (!Interactable)
		{
			return;
		}

		Interactable->GetTargetCancelInteractionDelegate(InteractingComponent.Get())->Remove(InteractionCanceledDelegateHandle);
	}

	Super::OnDestroy(AbilityEnded);
}
