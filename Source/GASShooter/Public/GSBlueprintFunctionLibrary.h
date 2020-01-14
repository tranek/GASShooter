// Copyright 2020 Dan Kestranek.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Characters/Abilities/GSAbilitySystemComponent.h"
#include "Characters/Abilities/GSGameplayAbility.h"
#include "Characters/Abilities/GSAbilityTypes.h"
#include "GSBlueprintFunctionLibrary.generated.h"


/**
 * 
 */
UCLASS()
class GASSHOOTER_API UGSBlueprintFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	
public:
	/**
	* GameplayAbility
	*/
	UFUNCTION(BlueprintCallable, Category = "Abilities")
	static UGSGameplayAbility* GetPrimaryAbilityInstanceFromHandle(UAbilitySystemComponent* AbilitySystemComponent, FGameplayAbilitySpecHandle Handle);

	UFUNCTION(BlueprintCallable, Category = "Abilities")
	static UGSGameplayAbility* GetPrimaryAbilityInstanceFromClass(UAbilitySystemComponent* AbilitySystemComponent, TSubclassOf<UGameplayAbility> InAbilityClass);


	/**
	* FGameplayAbilitySpecHandle
	*/
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Abilities")
	static bool IsAbilitySpecHandleValid(FGameplayAbilitySpecHandle Handle);


	/**
	* GameplayEffectContainerSpec
	*/

	// USTRUCTs cannot contain UFUNCTIONS so we make static functions here
	// Checks if spec has any effects
	UFUNCTION(BlueprintPure, Category = "Ability")
	static bool DoesEffectContainerSpecHaveEffects(const FGSGameplayEffectContainerSpec& ContainerSpec);

	// Checks if spec has any targets
	UFUNCTION(BlueprintPure, Category = "Ability")
	static bool DoesEffectContainerSpecHaveTargets(const FGSGameplayEffectContainerSpec& ContainerSpec);

	// Clears spec's targets
	UFUNCTION(BlueprintCallable, Category = "Ability")
	static void ClearEffectContainerSpecTargets(UPARAM(ref) FGSGameplayEffectContainerSpec& ContainerSpec);

	// Adds targets to a copy of the passed in effect container spec and returns it
	UFUNCTION(BlueprintCallable, Category = "Ability", Meta = (AutoCreateRefTerm = "TargetData, HitResults, TargetActors"))
	static void AddTargetsToEffectContainerSpec(UPARAM(ref) FGSGameplayEffectContainerSpec& ContainerSpec, const TArray<FGameplayAbilityTargetDataHandle>& TargetData, const TArray<FHitResult>& HitResults, const TArray<AActor*>& TargetActors);

	// Applies container spec that was made from an ability
	UFUNCTION(BlueprintCallable, Category = "Ability")
	static TArray<FActiveGameplayEffectHandle> ApplyExternalEffectContainerSpec(const FGSGameplayEffectContainerSpec& ContainerSpec);
};
