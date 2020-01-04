// Copyright 2020 Dan Kestranek.


#include "Characters/Abilities/AbilityTasks/GSAT_WaitTargetDataUsingActor.h"
#include "AbilitySystemComponent.h"

void UGSAT_WaitTargetDataUsingActor::Activate()
{
	// Replace AbilityTask_WaitTargetData::Activate to avoid destroying the TargetActor

	// Need to handle case where target actor was passed into task
	if (Ability && (TargetClass == nullptr))
	{
		if (TargetActor)
		{
			AGameplayAbilityTargetActor* SpawnedActor = TargetActor;
			TargetClass = SpawnedActor->GetClass();

			RegisterTargetDataCallbacks();


			if (IsPendingKill())
			{
				return;
			}

			if (ShouldSpawnTargetActor())
			{
				InitializeTargetActor(SpawnedActor);
				FinalizeTargetActor(SpawnedActor);

				// Note that the call to FinalizeTargetActor, this task could finish and our owning ability may be ended.
			}
			// Removed part that destroys the TargetActor on the server if it isn't set to generate data on the server
		}
		else
		{
			EndTask();
		}
	}
}

UGSAT_WaitTargetDataUsingActor* UGSAT_WaitTargetDataUsingActor::WaitTargetDataWithReusableActor(UGameplayAbility* OwningAbility, FName TaskInstanceName, TEnumAsByte<EGameplayTargetingConfirmation::Type> ConfirmationType, AGameplayAbilityTargetActor* InTargetActor)
{
	UGSAT_WaitTargetDataUsingActor* MyObj = NewAbilityTask<UGSAT_WaitTargetDataUsingActor>(OwningAbility, TaskInstanceName);		//Register for task list here, providing a given FName as a key
	MyObj->TargetClass = nullptr;
	MyObj->TargetActor = InTargetActor;
	MyObj->ConfirmationType = ConfirmationType;
	return MyObj;
}

void UGSAT_WaitTargetDataUsingActor::OnDestroy(bool AbilityEnded)
{
	if (TargetActor)
	{
		// Clear added callbacks
		TargetActor->TargetDataReadyDelegate.RemoveAll(this);
		TargetActor->CanceledDelegate.RemoveAll(this);

		AbilitySystemComponent->GenericLocalConfirmCallbacks.RemoveDynamic(TargetActor, &AGameplayAbilityTargetActor::ConfirmTargeting);
		AbilitySystemComponent->GenericLocalCancelCallbacks.RemoveDynamic(TargetActor, &AGameplayAbilityTargetActor::CancelTargeting);
	}

	// Skip AbilityTask_WaitTargetData::OnDestroy to avoid destroying the TargetActor
	Super::Super::OnDestroy(AbilityEnded);
}