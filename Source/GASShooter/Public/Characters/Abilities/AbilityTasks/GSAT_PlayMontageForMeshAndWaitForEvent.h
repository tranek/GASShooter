// Copyright 2024 Dan Kestranek.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/Tasks/AbilityTask.h"
#include "Animation/AnimInstance.h"
#include "GSAT_PlayMontageForMeshAndWaitForEvent.generated.h"

class UGSAbilitySystemComponent;

/** Delegate type used, EventTag and Payload may be empty if it came from the montage callbacks */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FGSPlayMontageForMeshAndWaitForEventDelegate, FGameplayTag, EventTag, FGameplayEventData, EventData);

/**
 * This task combines PlayMontageAndWait and WaitForEvent into one task, so you can wait for multiple types of activations such as from a melee combo
 * Much of this code is copied from one of those two ability tasks
 * This is a good task to look at as an example when creating game-specific tasks
 * It is expected that each game will have a set of game-specific tasks to do what they want
 */
UCLASS()
class GASSHOOTER_API UGSAT_PlayMontageForMeshAndWaitForEvent : public UAbilityTask
{
	GENERATED_BODY()
	
public:
	// Constructor and overrides
	UGSAT_PlayMontageForMeshAndWaitForEvent(const FObjectInitializer& ObjectInitializer);

	/**
	* The Blueprint node for this task, PlayMontageAndWaitForEvent, has some black magic from the plugin that automagically calls Activate()
	* inside of K2Node_LatentAbilityCall as stated in the AbilityTask.h. Ability logic written in C++ probably needs to call Activate() itself manually.
	*/
	virtual void Activate() override;
	virtual void ExternalCancel() override;
	virtual FString GetDebugString() const override;
	virtual void OnDestroy(bool AbilityEnded) override;

	/** The montage completely finished playing */
	UPROPERTY(BlueprintAssignable)
	FGSPlayMontageForMeshAndWaitForEventDelegate OnCompleted;

	/** The montage started blending out */
	UPROPERTY(BlueprintAssignable)
	FGSPlayMontageForMeshAndWaitForEventDelegate OnBlendOut;

	/** The montage was interrupted */
	UPROPERTY(BlueprintAssignable)
	FGSPlayMontageForMeshAndWaitForEventDelegate OnInterrupted;

	/** The ability task was explicitly cancelled by another ability */
	UPROPERTY(BlueprintAssignable)
	FGSPlayMontageForMeshAndWaitForEventDelegate OnCancelled;

	/** One of the triggering gameplay events happened */
	UPROPERTY(BlueprintAssignable)
	FGSPlayMontageForMeshAndWaitForEventDelegate EventReceived;

	/**
	 * Play a montage and wait for it end. If a gameplay event happens that matches EventTags (or EventTags is empty), the EventReceived delegate will fire with a tag and event data.
	 * If StopWhenAbilityEnds is true, this montage will be aborted if the ability ends normally. It is always stopped when the ability is explicitly cancelled.
	 * On normal execution, OnBlendOut is called when the montage is blending out, and OnCompleted when it is completely done playing
	 * OnInterrupted is called if another montage overwrites this, and OnCancelled is called if the ability or task is cancelled
	 *
	 * @param TaskInstanceName Set to override the name of this task, for later querying
	 * @param MontageToPlay The montage to play on the character
	 * @param EventTags Any gameplay events matching this tag will activate the EventReceived callback. If empty, all events will trigger callback
	 * @param Rate Change to play the montage faster or slower
	 * @param bStopWhenAbilityEnds If true, this montage will be aborted if the ability ends normally. It is always stopped when the ability is explicitly cancelled
	 * @param AnimRootMotionTranslationScale Change to modify size of root motion or set to 0 to block it entirely
	 */
	UFUNCTION(BlueprintCallable, Category = "Ability|Tasks", meta = (HidePin = "OwningAbility", DefaultToSelf = "OwningAbility", BlueprintInternalUseOnly = "TRUE"))
	static UGSAT_PlayMontageForMeshAndWaitForEvent* PlayMontageForMeshAndWaitForEvent(
			UGameplayAbility* OwningAbility,
			FName TaskInstanceName,
			USkeletalMeshComponent* Mesh,
			UAnimMontage* MontageToPlay,
			FGameplayTagContainer EventTags,
			float Rate = 1.f,
			FName StartSection = NAME_None,
			bool bStopWhenAbilityEnds = true,
			float AnimRootMotionTranslationScale = 1.f,
			bool bReplicateMontage = true,
			float OverrideBlendOutTimeForCancelAbility = -1.f,
			float OverrideBlendOutTimeForStopWhenEndAbility = -1.f);

private:
	// Mesh that the Montage is playing on. Must be owned by the AvatarActor.
	UPROPERTY()
	USkeletalMeshComponent* Mesh;

	/** Montage that is playing */
	UPROPERTY()
	UAnimMontage* MontageToPlay;

	/** List of tags to match against gameplay events */
	UPROPERTY()
	FGameplayTagContainer EventTags;

	/** Playback rate */
	UPROPERTY()
	float Rate;

	/** Section to start montage from */
	UPROPERTY()
	FName StartSection;

	/** Modifies how root motion movement to apply */
	UPROPERTY()
	float AnimRootMotionTranslationScale;

	/** Rather montage should be aborted if ability ends */
	UPROPERTY()
	bool bStopWhenAbilityEnds;

	UPROPERTY()
	bool bReplicateMontage;

	UPROPERTY()
	float OverrideBlendOutTimeForCancelAbility;

	UPROPERTY()
	float OverrideBlendOutTimeForStopWhenEndAbility;

	/** Checks if the ability is playing a montage and stops that montage, returns true if a montage was stopped, false if not. */
	bool StopPlayingMontage(float OverrideBlendOutTime = -1.f);

	/** Returns our ability system component */
	UGSAbilitySystemComponent* GetTargetASC();

	void OnMontageBlendingOut(UAnimMontage* Montage, bool bInterrupted);
	void OnAbilityCancelled();
	void OnMontageEnded(UAnimMontage* Montage, bool bInterrupted);
	void OnGameplayEvent(FGameplayTag EventTag, const FGameplayEventData* Payload);

	FOnMontageBlendingOutStarted BlendingOutDelegate;
	FOnMontageEnded MontageEndedDelegate;
	FDelegateHandle CancelledHandle;
	FDelegateHandle EventHandle;
	
};
