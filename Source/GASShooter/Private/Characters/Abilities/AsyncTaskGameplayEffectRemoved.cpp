// Copyright 2020 Dan Kestranek.


#include "Characters/Abilities/AsyncTaskGameplayEffectRemoved.h"
#include "GSBlueprintFunctionLibrary.h"

UAsyncTaskGameplayEffectRemoved* UAsyncTaskGameplayEffectRemoved::ListenForGameplayEffectRemoved(UAbilitySystemComponent * AbilitySystemComponent)
{
	UAsyncTaskGameplayEffectRemoved* ListenForGameplayEffectRemoved = NewObject<UAsyncTaskGameplayEffectRemoved>();
	ListenForGameplayEffectRemoved->ASC = AbilitySystemComponent;

	if (!IsValid(AbilitySystemComponent))
	{
		ListenForGameplayEffectRemoved->EndTask();
		return nullptr;
	}

	AbilitySystemComponent->OnAnyGameplayEffectRemovedDelegate().AddUObject(ListenForGameplayEffectRemoved, &UAsyncTaskGameplayEffectRemoved::GameplayEffectRemoved);

	return ListenForGameplayEffectRemoved;
}

void UAsyncTaskGameplayEffectRemoved::EndTask()
{
	if (IsValid(ASC))
	{
		ASC->OnAnyGameplayEffectRemovedDelegate().RemoveAll(this);
	}

	SetReadyToDestroy();
	MarkPendingKill();
}

void UAsyncTaskGameplayEffectRemoved::GameplayEffectRemoved(const FActiveGameplayEffect& EffectRemoved)
{
	OnGameplayEffectRemoved.Broadcast(EffectRemoved);
}
