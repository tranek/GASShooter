// Copyright 2020 Dan Kestranek.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemGlobals.h"
#include "GSAbilitySystemGlobals.generated.h"

/**
 * 
 */
UCLASS()
class GASSHOOTER_API UGSAbilitySystemGlobals : public UAbilitySystemGlobals
{
	GENERATED_BODY()
	
public:
	UGSAbilitySystemGlobals();

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
