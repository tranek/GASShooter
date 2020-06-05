// Copyright 2020 Dan Kestranek.


#include "Characters/Abilities/AsyncTaskGameplayTagChanged.h"
#include "GSBlueprintFunctionLibrary.h"

UAsyncTaskGameplayTagChanged* UAsyncTaskGameplayTagChanged::ListenForGameplayTagChanged(UAbilitySystemComponent * AbilitySystemComponent, FGameplayTagContainer InTags)
{
	UAsyncTaskGameplayTagChanged* ListenForGameplayTagChanged = NewObject<UAsyncTaskGameplayTagChanged>();
	ListenForGameplayTagChanged->ASC = AbilitySystemComponent;
	ListenForGameplayTagChanged->Tags = InTags;

	if (!IsValid(AbilitySystemComponent) || InTags.Num() < 1)
	{
		ListenForGameplayTagChanged->EndTask();
		return nullptr;
	}

	TArray<FGameplayTag> TagArray;
	InTags.GetGameplayTagArray(TagArray);
	
	for (FGameplayTag Tag : TagArray)
	{
		AbilitySystemComponent->RegisterGameplayTagEvent(Tag, EGameplayTagEventType::AnyCountChange).AddUObject(ListenForGameplayTagChanged, &UAsyncTaskGameplayTagChanged::TagChanged);
	}

	return ListenForGameplayTagChanged;
}

void UAsyncTaskGameplayTagChanged::EndTask()
{
	if (IsValid(ASC))
	{
		TArray<FGameplayTag> TagArray;
		Tags.GetGameplayTagArray(TagArray);

		for (FGameplayTag Tag : TagArray)
		{
			ASC->RegisterGameplayTagEvent(Tag, EGameplayTagEventType::AnyCountChange).RemoveAll(this);
		}
	}

	SetReadyToDestroy();
	MarkPendingKill();
}

void UAsyncTaskGameplayTagChanged::TagChanged(const FGameplayTag Tag, int32 NewCount)
{
	OnTagChanged.Broadcast(Tag, NewCount);
}
