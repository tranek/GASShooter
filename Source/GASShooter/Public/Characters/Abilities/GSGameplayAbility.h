// Copyright 2019 Dan Kestranek.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "GASShooter.h"
#include "GSGameplayAbility.generated.h"

/**
 * 
 */
UCLASS()
class GASSHOOTER_API UGSGameplayAbility : public UGameplayAbility
{
	GENERATED_BODY()

public:
	UGSGameplayAbility();

	// Abilities with this set will automatically activate when the input is pressed
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Ability")
	EGSAbilityInputID AbilityInputID = EGSAbilityInputID::None;

	// Value to associate an ability with an slot without tying it to an automatically activated input.
	// Passive abilities won't be tied to an input so we need a way to generically associate abilities with slots.
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Ability")
	EGSAbilityInputID AbilityID = EGSAbilityInputID::None;

	// Tells an ability to activate immediately when its granted. Used for passive abilities and abilites forced on others.
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Ability")
	bool ActivateAbilityOnGranted = false;

	// If an ability is marked as 'ActivateAbilityOnGranted', activate them immediately when given here
	// Epic's comment: Projects may want to initiate passives or do other "BeginPlay" type of logic here.
	virtual void OnAvatarSet(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec) override;
};
