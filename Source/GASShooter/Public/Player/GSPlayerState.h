// Copyright 2019 Dan Kestranek.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "AbilitySystemInterface.h"
#include "GameplayEffectTypes.h"
#include "GSPlayerState.generated.h"

/**
 * 
 */
UCLASS()
class GASSHOOTER_API AGSPlayerState : public APlayerState, public IAbilitySystemInterface
{
	GENERATED_BODY()
	
public:
	AGSPlayerState();

	// Implement IAbilitySystemInterface
	class UAbilitySystemComponent* GetAbilitySystemComponent() const override;

	class UGSAttributeSetBase* GetAttributeSetBase() const;

	UFUNCTION(BlueprintCallable, Category = "GASShooter|GSPlayerState")
	bool IsAlive() const;

	/**
	* Getters for attributes from GDAttributeSetBase. Returns Current Value unless otherwise specified.
	*/

	UFUNCTION(BlueprintCallable, Category = "GASShooter|GSPlayerState|Attributes")
	float GetHealth() const;

	UFUNCTION(BlueprintCallable, Category = "GASShooter|GSPlayerState|Attributes")
	float GetMaxHealth() const;

	UFUNCTION(BlueprintCallable, Category = "GASShooter|GSPlayerState|Attributes")
	float GetHealthRegenRate() const;

	UFUNCTION(BlueprintCallable, Category = "GASShooter|GSPlayerState|Attributes")
	float GetMana() const;

	UFUNCTION(BlueprintCallable, Category = "GASShooter|GSPlayerState|Attributes")
	float GetMaxMana() const;

	UFUNCTION(BlueprintCallable, Category = "GASShooter|GSPlayerState|Attributes")
	float GetManaRegenRate() const;

	UFUNCTION(BlueprintCallable, Category = "GASShooter|GSPlayerState|Attributes")
	float GetStamina() const;

	UFUNCTION(BlueprintCallable, Category = "GASShooter|GSPlayerState|Attributes")
	float GetMaxStamina() const;

	UFUNCTION(BlueprintCallable, Category = "GASShooter|GSPlayerState|Attributes")
	float GetStaminaRegenRate() const;

	UFUNCTION(BlueprintCallable, Category = "GASShooter|GSPlayerState|Attributes")
	float GetArmor() const;

	UFUNCTION(BlueprintCallable, Category = "GASShooter|GSPlayerState|Attributes")
	float GetMoveSpeed() const;

	UFUNCTION(BlueprintCallable, Category = "GASShooter|GSPlayerState|Attributes")
	int32 GetCharacterLevel() const;

	UFUNCTION(BlueprintCallable, Category = "GASShooter|GSPlayerState|Attributes")
	int32 GetXP() const;

	UFUNCTION(BlueprintCallable, Category = "GASShooter|GSPlayerState|Attributes")
	int32 GetXPBounty() const;

	UFUNCTION(BlueprintCallable, Category = "GASShooter|GSPlayerState|Attributes")
	int32 GetGold() const;

	UFUNCTION(BlueprintCallable, Category = "GASShooter|GSPlayerState|Attributes")
	int32 GetGoldBounty() const;

protected:
	UPROPERTY()
	class UGSAbilitySystemComponent* AbilitySystemComponent;

	UPROPERTY()
	class UGSAttributeSetBase* AttributeSetBase;

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
};
