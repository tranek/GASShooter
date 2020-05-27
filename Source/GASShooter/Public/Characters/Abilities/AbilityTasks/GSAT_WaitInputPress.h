// Copyright 2020 Dan Kestranek.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/Tasks/AbilityTask.h"
#include "GSAT_WaitInputPress.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FWaitInputPressedDelegate);

/**
 * Waits until the input is pressed from activating an ability. This should be true immediately upon starting the ability, since the key was pressed to activate it.
 * We expect server to execute this task in parallel and keep its own time. This can fire more than once and does not keep track of how long it took to press input.
 */
UCLASS()
class GASSHOOTER_API UGSAT_WaitInputPress : public UAbilityTask
{
	GENERATED_UCLASS_BODY()

	UPROPERTY(BlueprintAssignable)
	FWaitInputPressedDelegate OnPress;

	/**
	* Wait until the user presses the input button for this ability's activation.
	*/
	UFUNCTION(BlueprintCallable, Category = "Ability|Tasks", meta = (HidePin = "OwningAbility", DefaultToSelf = "OwningAbility", BlueprintInternalUseOnly = "TRUE"))
	static UGSAT_WaitInputPress* WaitSimpleInputPress(UGameplayAbility* OwningAbility, bool bTestAlreadyPressed = false, bool bTriggerOnce = false);

	virtual void Activate() override;

	UFUNCTION()
	void OnPressCallback();

protected:
	bool bTestInitialState;
	bool bTriggerOnce;
	FDelegateHandle DelegateHandle;

	virtual void OnDestroy(bool AbilityEnded) override;
};
