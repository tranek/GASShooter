// Copyright 2020 Dan Kestranek.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "AbilitySystemComponent.h"
#include "GameplayTagContainer.h"
#include "AsyncTaskGameplayEffectRemoved.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnGameplayEffectRemoved, const FActiveGameplayEffect&, EffectRemoved);

/**
 * Blueprint node to automatically register a listener for when GameplayEffects are removed.
 * Useful to use in Blueprint/UMG.
 */
UCLASS(BlueprintType, meta = (ExposedAsyncProxy = AsyncTask))
class GASSHOOTER_API UAsyncTaskGameplayEffectRemoved : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()
	
public:
	UPROPERTY(BlueprintAssignable)
	FOnGameplayEffectRemoved OnGameplayEffectRemoved;

	// Listens for when GameplayEffects are removed.
	UFUNCTION(BlueprintCallable, meta = (BlueprintInternalUseOnly = "true"))
	static UAsyncTaskGameplayEffectRemoved* ListenForGameplayEffectRemoved(UAbilitySystemComponent* AbilitySystemComponent);

	// You must call this function manually when you want the AsyncTask to end.
	// For UMG Widgets, you would call it in the Widget's Destruct event.
	UFUNCTION(BlueprintCallable)
	void EndTask();

protected:
	UPROPERTY()
	UAbilitySystemComponent* ASC;

	virtual void GameplayEffectRemoved(const FActiveGameplayEffect& EffectRemoved);
};
