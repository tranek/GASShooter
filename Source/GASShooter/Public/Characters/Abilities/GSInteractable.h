// Copyright 2020 Dan Kestranek.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
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
	// Is this object available for player interaction at right now?
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interactable")
	bool IsAvailableForInteraction() const;
	virtual bool IsAvailableForInteraction_Implementation() const;

	// How long does the player need to hold down the interact button to interact with this?
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interactable")
	float GetInteractDuration() const;
	virtual float GetInteractDuration_Implementation() const;

	/**
	* Should the Interact ability sync with the Server before calling PreInteract()? This will stall calling PreInteract()
	* from starting until the client syncs with the Server.
	*
	* Player revive sets this to true so that the player reviving is in sync with the server since we can't locally
	* predict an ability run on another player. The downed player's reviving animation will be in sync with the local
	* player's Interact Duration Timer.
	*/
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interactable")
	bool ClientShouldSyncPreInteract() const;
	virtual bool ClientShouldSyncPreInteract_Implementation() const = 0;

	/**
	* Interact with this Actor. This will call before starting the Interact Duration timer. This might do things, apply
	* (predictively or not) GameplayEffects, trigger (predictively or not) GameplayAbilities, etc.
	*
	* You can use this function to grant abilities that will be predictively activated on PostInteract() to hide the
	* AbilitySpec replication time.
	*
	* Player revives use PreInteract() to trigger a ability that plays an animation that lasts the same duration as
	* the Interact Duration. If this ability finishes, it will revive the player in PostInteract().
	*
	* @param InteractingActor The Actor interacting with this Actor. It will be the AvatarActor from a GameplayAbility.
	*/
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interactable")
	void PreInteract(AActor* InteractingActor);
	virtual void PreInteract_Implementation(AActor* InteractingActor) {};

	/**
	* Interact with this Actor. This will call after the Interact Duration timer completes. This might do things, apply
	* (predictively or not) GameplayEffects, trigger (predictively or not) GameplayAbilities, etc.
	*
	* If you need to trigger a GameplayAbility predictively, the player's ASC needs to have been granted the ability
	* ahead of time. If you don't want to grant every possible predictive ability at game start, you can hide the time
	* needed to replicate the AbilitySpec inside the time needed to interact by granted it in PreInteract().
	*
	* @param InteractingActor The Actor interacting with this Actor. It will be the AvatarActor from a GameplayAbility.
	*/
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interactable")
	void PostInteract(AActor* InteractingActor);
	virtual void PostInteract_Implementation(AActor* InteractingActor) {};

	/**
	* Cancel an ongoing interaction, typically anything happening in PreInteract() while waiting on the Interact Duration
	* Timer. This will be called if the player releases input early.
	*/
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interactable")
	void CancelInteraction();
	virtual void CancelInteraction_Implementation() {};
};
