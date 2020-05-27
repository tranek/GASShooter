// Copyright 2020 Dan Kestranek.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/Tasks/AbilityTask.h"
#include "GSAT_WaitInputPressWithTags.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FInputPressWithTagsDelegate, float, TimeWaited);

/**
 * Waits until the input is pressed from activating an ability and the ASC has the required tags and not the ignored tags.
 * This should be true immediately upon starting the ability, since the key was pressed to activate it. We expect server to
 * execute this task in parallel and keep its own time.
 */
UCLASS()
class GASSHOOTER_API UGSAT_WaitInputPressWithTags : public UAbilityTask
{
	GENERATED_UCLASS_BODY()
	
	UPROPERTY(BlueprintAssignable)
	FInputPressWithTagsDelegate OnPress;

	/**
	* Wait until the user presses the input button for this ability's activation. Returns time this node spent waiting for the press. Will return 0 if input was already down.
	*
	* @param RequiredTags Ability Owner must have all of these tags otherwise the input is ignored.
	* @param IgnoredTags Ability Owner cannot have any of these tags otherwise the input is ignored.
	*/
	UFUNCTION(BlueprintCallable, Category = "Ability|Tasks", meta = (HidePin = "OwningAbility", DefaultToSelf = "OwningAbility", BlueprintInternalUseOnly = "TRUE"))
	static UGSAT_WaitInputPressWithTags* WaitInputPressWithTags(UGameplayAbility* OwningAbility, FGameplayTagContainer RequiredTags, FGameplayTagContainer IgnoredTags, bool bTestAlreadyPressed = false);

	virtual void Activate() override;

	UFUNCTION()
	void OnPressCallback();

protected:
	float StartTime;

	bool bTestInitialState;

	FDelegateHandle DelegateHandle;

	FGameplayTagContainer RequiredTags;
	FGameplayTagContainer IgnoredTags;

	virtual void OnDestroy(bool AbilityEnded) override;

	/**
	* We can only listen for one input pressed event. I think it's because
	* UAbilitySystemComponent::InvokeReplicatedEvent sets ReplicatedData->GenericEvents[(uint8)EventType].bTriggered = true;
	* So if we want to keep listening for more input events, we just clear the delegate handle and bind again.
	*/
	virtual void Reset();
};
