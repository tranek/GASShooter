// Copyright 2020 Dan Kestranek.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemGlobals.h"
#include "GSAbilitySystemGlobals.generated.h"

/**
 * Child class of UAbilitySystemGlobals.
 * Do not try to get a reference to this or call into it during constructors of other UObjects. It will crash in packaged games.
 */
UCLASS()
class GASSHOOTER_API UGSAbilitySystemGlobals : public UAbilitySystemGlobals
{
	GENERATED_BODY()
	
public:
	UGSAbilitySystemGlobals();

	/**
	* Cache commonly used tags here. This has the benefit of one place to set the tag FName in case tag names change and
	* the function call into UGSAbilitySystemGlobals::GSGet() is cheaper than calling FGameplayTag::RequestGameplayTag().
	* Classes can access them by UGSAbilitySystemGlobals::GSGet().DeadTag
	* We're not using this in this sample project (classes are manually caching in their constructors), but it's here as a reference.
	*/

	UPROPERTY()
	FGameplayTag DeadTag;

	UPROPERTY()
	FGameplayTag KnockedDownTag;

	UPROPERTY()
	FGameplayTag InteractingTag;

	UPROPERTY()
	FGameplayTag InteractingRemovalTag;

	static UGSAbilitySystemGlobals& GSGet()
	{
		return dynamic_cast<UGSAbilitySystemGlobals&>(Get());
	}

	/** Should allocate a project specific GameplayEffectContext struct. Caller is responsible for deallocation */
	virtual FGameplayEffectContext* AllocGameplayEffectContext() const override;

	virtual void InitGlobalTags() override;
};
