// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbilityTargetActor_SingleLineTrace.h"
#include "GSGATA_SingleLineTrace.generated.h"

/**
 * 
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

	virtual void CancelTargeting() override;
};
