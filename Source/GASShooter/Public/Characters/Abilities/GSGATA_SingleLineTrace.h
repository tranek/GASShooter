// Copyright 2020 Dan Kestranek.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbilityTargetActor_SingleLineTrace.h"
#include "GSGATA_SingleLineTrace.generated.h"

/**
 * Reusable single target line trace target actor. Stops tracing at the first blocking hit.
 */
UCLASS()
class GASSHOOTER_API AGSGATA_SingleLineTrace : public AGameplayAbilityTargetActor_SingleLineTrace
{
	GENERATED_BODY()
	
public:
	AGSGATA_SingleLineTrace();

	// Expose to Blueprint
	UFUNCTION(BlueprintCallable)
	void SetStartLocation(const FGameplayAbilityTargetingLocationInfo& InStartLocation);

	// Expose to Blueprint
	UFUNCTION(BlueprintCallable)
	void SetShouldProduceTargetDataOnServer(bool InShouldProduceTargetDataOnServer);

	// Expose to Blueprint
	UFUNCTION(BlueprintCallable)
	void SetDestroyOnConfirmation(bool DestroyOnConfirmation = false);

	virtual void StartTargeting(UGameplayAbility* Ability) override;

	virtual void CancelTargeting() override;

	virtual void BeginPlay() override;
};
