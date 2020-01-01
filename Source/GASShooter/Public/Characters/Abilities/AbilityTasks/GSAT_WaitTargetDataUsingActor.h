// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/Tasks/AbilityTask_WaitTargetData.h"
#include "GSAT_WaitTargetDataUsingActor.generated.h"

/**
 * Waits for TargetData from an already existing TargetActor and does *NOT* destroy it when it receives data.
 */
UCLASS()
class GASSHOOTER_API UGSAT_WaitTargetDataUsingActor : public UAbilityTask_WaitTargetData
{
	GENERATED_BODY()
	
public:
	virtual void Activate() override;

	// Uses specified target actor and waits for it to return valid data or to be canceled.
	UFUNCTION(BlueprintCallable, meta = (HidePin = "OwningAbility", DefaultToSelf = "OwningAbility", BlueprintInternalUseOnly = "true", HideSpawnParms = "Instigator"), Category = "Ability|Tasks")
	static UGSAT_WaitTargetDataUsingActor* WaitTargetDataWithReusableActor(UGameplayAbility* OwningAbility, FName TaskInstanceName, TEnumAsByte<EGameplayTargetingConfirmation::Type> ConfirmationType, AGameplayAbilityTargetActor* InTargetActor);

protected:
	virtual void OnDestroy(bool AbilityEnded) override;
};
