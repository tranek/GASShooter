// Copyright 2020 Dan Kestranek.


#include "Characters/Abilities/AsyncTaskGameplayEffectAdded.h"
#include "GSBlueprintFunctionLibrary.h"

UAsyncTaskGameplayEffectAdded* UAsyncTaskGameplayEffectAdded::ListenForGameplayEffectAdded(UAbilitySystemComponent * AbilitySystemComponent)
{
	UAsyncTaskGameplayEffectAdded* ListenForGameplayEffectAdded = NewObject<UAsyncTaskGameplayEffectAdded>();
	ListenForGameplayEffectAdded->ASC = AbilitySystemComponent;

	if (!IsValid(AbilitySystemComponent))
	{
		ListenForGameplayEffectAdded->EndTask();
		return nullptr;
	}

	AbilitySystemComponent->OnGameplayEffectAppliedDelegateToSelf.AddUObject(ListenForGameplayEffectAdded, &UAsyncTaskGameplayEffectAdded::GameplayEffectAdded);

	return ListenForGameplayEffectAdded;
}

void UAsyncTaskGameplayEffectAdded::EndTask()
{
	if (IsValid(ASC))
	{
		ASC->OnGameplayEffectAppliedDelegateToSelf.RemoveAll(this);
	}

	SetReadyToDestroy();
	MarkPendingKill();
}

void UAsyncTaskGameplayEffectAdded::GameplayEffectAdded(UAbilitySystemComponent* Target, const FGameplayEffectSpec& SpecApplied, FActiveGameplayEffectHandle ActiveHandle)
{
	OnGameplayEffectAdded.Broadcast(Target, SpecApplied, ActiveHandle);
}
