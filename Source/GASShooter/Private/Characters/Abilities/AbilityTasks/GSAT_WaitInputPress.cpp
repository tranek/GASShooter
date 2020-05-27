// Copyright 2020 Dan Kestranek.


#include "Characters/Abilities/AbilityTasks/GSAT_WaitInputPress.h"
#include "AbilitySystemComponent.h"

UGSAT_WaitInputPress::UGSAT_WaitInputPress(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	bTestInitialState = false;
	bTriggerOnce = false;
}

UGSAT_WaitInputPress* UGSAT_WaitInputPress::WaitSimpleInputPress(UGameplayAbility* OwningAbility, bool bTestAlreadyPressed, bool bTriggerOnce)
{
	UGSAT_WaitInputPress* Task = NewAbilityTask<UGSAT_WaitInputPress>(OwningAbility);
	Task->bTestInitialState = bTestAlreadyPressed;
	Task->bTriggerOnce = bTriggerOnce;
	return Task;
}

void UGSAT_WaitInputPress::Activate()
{
	if (Ability)
	{
		if (bTestInitialState && IsLocallyControlled())
		{
			FGameplayAbilitySpec* Spec = Ability->GetCurrentAbilitySpec();
			if (Spec && Spec->InputPressed)
			{
				OnPressCallback();
				return;
			}
		}

		DelegateHandle = AbilitySystemComponent->AbilityReplicatedEventDelegate(EAbilityGenericReplicatedEvent::InputPressed, GetAbilitySpecHandle(), GetActivationPredictionKey()).AddUObject(this, &UGSAT_WaitInputPress::OnPressCallback);
		if (IsForRemoteClient())
		{
			if (!AbilitySystemComponent->CallReplicatedEventDelegateIfSet(EAbilityGenericReplicatedEvent::InputPressed, GetAbilitySpecHandle(), GetActivationPredictionKey()))
			{
				SetWaitingOnRemotePlayerData();
			}
		}
	}
}

void UGSAT_WaitInputPress::OnPressCallback()
{
	if (!Ability || !AbilitySystemComponent)
	{
		return;
	}

	AbilitySystemComponent->AbilityReplicatedEventDelegate(EAbilityGenericReplicatedEvent::InputPressed, GetAbilitySpecHandle(), GetActivationPredictionKey()).Remove(DelegateHandle);

	FScopedPredictionWindow ScopedPrediction(AbilitySystemComponent, IsPredictingClient());

	if (IsPredictingClient())
	{
		// Tell the server about this
		AbilitySystemComponent->ServerSetReplicatedEvent(EAbilityGenericReplicatedEvent::InputPressed, GetAbilitySpecHandle(), GetActivationPredictionKey(), AbilitySystemComponent->ScopedPredictionKey);
	}
	else
	{
		AbilitySystemComponent->ConsumeGenericReplicatedEvent(EAbilityGenericReplicatedEvent::InputPressed, GetAbilitySpecHandle(), GetActivationPredictionKey());
	}

	if (ShouldBroadcastAbilityTaskDelegates())
	{
		OnPress.Broadcast();
	}

	if (bTriggerOnce)
	{
		EndTask();
	}
}

void UGSAT_WaitInputPress::OnDestroy(bool AbilityEnded)
{
	AbilitySystemComponent->AbilityReplicatedEventDelegate(EAbilityGenericReplicatedEvent::InputPressed, GetAbilitySpecHandle(), GetActivationPredictionKey()).Remove(DelegateHandle);

	Super::OnDestroy(AbilityEnded);
}
