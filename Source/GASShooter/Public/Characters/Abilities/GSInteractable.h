// Copyright 2020 Dan Kestranek.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "Abilities/Tasks/AbilityTask_NetworkSyncPoint.h"
#include "GSInteractable.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UGSInteractable : public UInterface
{
	GENERATED_BODY()
};

/**
 * Interface for Actors that can be interacted with through the GameplayAbilitySystem.
 */
class GASSHOOTER_API IGSInteractable
{
	GENERATED_BODY()

		// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	/**
	* Is this object available for player interaction at right now?
	*
	* @param InteractionComponent Optional UPrimitiveComponent in case an Actor has many separate interactable areas.
	*/
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interactable")
	bool IsAvailableForInteraction(UPrimitiveComponent* InteractionComponent = nullptr) const;
	virtual bool IsAvailableForInteraction_Implementation(UPrimitiveComponent* InteractionComponent = nullptr) const;

	/**
	* How long does the player need to hold down the interact button to interact with this?
	*
	* @param InteractionComponent Optional UPrimitiveComponent in case an Actor has many separate interactable areas.
	*/
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interactable")
	float GetInteractDuration(UPrimitiveComponent* InteractionComponent = nullptr) const;
	virtual float GetInteractDuration_Implementation(UPrimitiveComponent* InteractionComponent = nullptr) const;

	/**
	* Should we sync and who should sync before calling PreInteract()? Defaults to false and OnlyServerWait.
	* OnlyServerWait - client predictively calls PreInteract().
	* OnlyClientWait - client waits for server to call PreInteract(). This is useful if we are activating an ability
	* on another ASC (player) and want to sync actions or animations with our Interact Duration timer.
	* BothWait - client and server wait for each other before calling PreInteract().
	*
	* Player revive uses OnlyClientWait so that the player reviving is in sync with the server since we can't locally
	* predict an ability run on another player. The downed player's reviving animation will be in sync with the local
	* player's Interact Duration Timer.
	*
	* @param InteractionComponent Optional UPrimitiveComponent in case an Actor has many separate interactable areas.
	*/
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interactable")
	void GetPreInteractSyncType(bool& bShouldSync, EAbilityTaskNetSyncType& Type, UPrimitiveComponent* InteractionComponent = nullptr) const;
	void GetPreInteractSyncType_Implementation(bool& bShouldSync, EAbilityTaskNetSyncType& Type, UPrimitiveComponent* InteractionComponent = nullptr) const;

	/**
	* Should we sync and who should sync before calling PostInteract()? Defaults to false and OnlyServerWait.
	* OnlyServerWait - client predictively calls PostInteract().
	* OnlyClientWait - client waits for server to call PostInteract().
	* BothWait - client and server wait for each other before calling PostInteract().
	*
	* Player revive uses OnlyServerWait so that the client isn't stuck waiting for the server after the Interaction Duration
	* ends. Revive's PostInteract() will only run code on the server so it's fine for the client to be "finished" ahead of
	* the server.
	*
	* @param InteractionComponent Optional UPrimitiveComponent in case an Actor has many separate interactable areas.
	*/
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interactable")
	void GetPostInteractSyncType(bool& bShouldSync, EAbilityTaskNetSyncType& Type, UPrimitiveComponent* InteractionComponent = nullptr) const;
	void GetPostInteractSyncType_Implementation(bool& bShouldSync, EAbilityTaskNetSyncType& Type, UPrimitiveComponent* InteractionComponent = nullptr) const;

	/**
	* Interact with this Actor. This will call before starting the Interact Duration timer. This might do things, apply
	* (predictively or not) GameplayEffects, trigger (predictively or not) GameplayAbilities, etc.
	*
	* You can use this function to grant abilities that will be predictively activated on PostInteract() to hide the
	* AbilitySpec replication time.
	*
	* If you want to do something predictively, you can get the ASC from the InteractingActor and use its
	* ScopedPredictionKey.
	*
	* Player revives use PreInteract() to trigger a ability that plays an animation that lasts the same duration as
	* the Interact Duration. If this ability finishes, it will revive the player in PostInteract().
	*
	* @param InteractingActor The Actor interacting with this Actor. It will be the AvatarActor from a GameplayAbility.
	* @param InteractionComponent Optional UPrimitiveComponent in case an Actor has many separate interactable areas.
	*/
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interactable")
	void PreInteract(AActor* InteractingActor, UPrimitiveComponent* InteractionComponent = nullptr);
	virtual void PreInteract_Implementation(AActor* InteractingActor, UPrimitiveComponent* InteractionComponent = nullptr) {};

	/**
	* Interact with this Actor. This will call after the Interact Duration timer completes. This might do things, apply
	* (predictively or not) GameplayEffects, trigger (predictively or not) GameplayAbilities, etc.
	*
	* If you want to do something predictively, you can get the ASC from the InteractingActor and use its
	* ScopedPredictionKey.
	*
	* If you need to trigger a GameplayAbility predictively, the player's ASC needs to have been granted the ability
	* ahead of time. If you don't want to grant every possible predictive ability at game start, you can hide the time
	* needed to replicate the AbilitySpec inside the time needed to interact by granted it in PreInteract().
	*
	* @param InteractingActor The Actor interacting with this Actor. It will be the AvatarActor from a GameplayAbility.
	* @param InteractionComponent Optional UPrimitiveComponent in case an Actor has many separate interactable areas.
	*/
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interactable")
	void PostInteract(AActor* InteractingActor, UPrimitiveComponent* InteractionComponent = nullptr);
	virtual void PostInteract_Implementation(AActor* InteractingActor, UPrimitiveComponent* InteractionComponent = nullptr) {};

	/**
	* Cancel an ongoing interaction, typically anything happening in PreInteract() while waiting on the Interact Duration
	* Timer. This will be called if the player releases input early.
	*
	* @param InteractionComponent Optional UPrimitiveComponent in case an Actor has many separate interactable areas.
	*/
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interactable")
	void CancelInteraction(UPrimitiveComponent* InteractionComponent = nullptr);
	virtual void CancelInteraction_Implementation(UPrimitiveComponent* InteractionComponent = nullptr) {};

	/**
	* Returns a delegate for GA_Interact to bind to that fires when this Actor is canceling the interactiong (e.g. died).
	*
	* @param InteractionComponent Optional UPrimitiveComponent in case an Actor has many separate interactable areas.
	*/
	virtual FSimpleMulticastDelegate* GetTargetCancelInteractionDelegate(UPrimitiveComponent* InteractionComponent = nullptr);
};
