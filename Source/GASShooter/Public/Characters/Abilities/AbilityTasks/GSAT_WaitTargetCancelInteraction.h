// Copyright 2020 Dan Kestranek.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/Tasks/AbilityTask.h"
#include "GSAT_WaitTargetCancelInteraction.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FWaitTargetCancelInteractionDelegate);

/**
 * AbilityTask waits for the interaction target to cancel being interacted with (e.g. a reviving player is killed before
 * it finished reviving).
 */
UCLASS()
class GASSHOOTER_API UGSAT_WaitTargetCancelInteraction : public UAbilityTask
{
	GENERATED_UCLASS_BODY()

	UPROPERTY(BlueprintAssignable)
	FWaitTargetCancelInteractionDelegate InteractionCanceled;

	/**
	* Waits for the interactoin target to cancel the interaction (e.g. a reviving player is killed before it finished reviving).
	*/
	UFUNCTION(BlueprintCallable, meta = (HidePin = "OwningAbility", DefaultToSelf = "OwningAbility", BlueprintInternalUseOnly = "true", HideSpawnParms = "Instigator"), Category = "Ability|Tasks")
	static UGSAT_WaitTargetCancelInteraction* WaitForTargetCancelInteraction(UGameplayAbility* OwningAbility, UPrimitiveComponent* InteractingComponent);

	virtual void Activate() override;

	virtual void OnInteractionCanceled();

protected:
	TWeakObjectPtr<UPrimitiveComponent> InteractingComponent;

	FDelegateHandle InteractionCanceledDelegateHandle;

	virtual void OnDestroy(bool AbilityEnded) override;
};
