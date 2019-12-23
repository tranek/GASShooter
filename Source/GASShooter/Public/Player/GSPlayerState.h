// Copyright 2019 Dan Kestranek.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "AbilitySystemInterface.h"
#include "GameplayEffectTypes.h"
#include "GSPlayerState.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FGSOnGameplayAttributeValueChangedDelegate, FGameplayAttribute, Attribute, float, NewValue, float, OldValue);

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

	// Attribute changed delegate handles
	FDelegateHandle HealthChangedDelegateHandle;
	/*
	FDelegateHandle MaxHealthChangedDelegateHandle;
	FDelegateHandle HealthRegenRateChangedDelegateHandle;
	FDelegateHandle ManaChangedDelegateHandle;
	FDelegateHandle MaxManaChangedDelegateHandle;
	FDelegateHandle ManaRegenRateChangedDelegateHandle;
	FDelegateHandle StaminaChangedDelegateHandle;
	FDelegateHandle MaxStaminaChangedDelegateHandle;
	FDelegateHandle StaminaRegenRateChangedDelegateHandle;
	FDelegateHandle XPChangedDelegateHandle;
	FDelegateHandle GoldChangedDelegateHandle;
	FDelegateHandle CharacterLevelChangedDelegateHandle;
	*/

	/*
	// Attribute changed delegates for Blueprint
	UPROPERTY(BlueprintAssignable, Category = "GASShooter|GSPlayerState|Attributes")
	FGSOnGameplayAttributeValueChangedDelegate OnHealthAttributeChanged;
	UPROPERTY(BlueprintAssignable, Category = "GASShooter|GSPlayerState|Attributes")
	FGSOnGameplayAttributeValueChangedDelegate OnMaxHealthAttributeChanged;
	UPROPERTY(BlueprintAssignable, Category = "GASShooter|GSPlayerState|Attributes")
	FGSOnGameplayAttributeValueChangedDelegate OnHealthRegenRateAttributeChanged;
	UPROPERTY(BlueprintAssignable, Category = "GASShooter|GSPlayerState|Attributes")
	FGSOnGameplayAttributeValueChangedDelegate OnManaAttributeChanged;
	UPROPERTY(BlueprintAssignable, Category = "GASShooter|GSPlayerState|Attributes")
	FGSOnGameplayAttributeValueChangedDelegate OnMaxManaAttributeChanged;
	UPROPERTY(BlueprintAssignable, Category = "GASShooter|GSPlayerState|Attributes")
	FGSOnGameplayAttributeValueChangedDelegate OnManaRegenRateAttributeChanged;
	UPROPERTY(BlueprintAssignable, Category = "GASShooter|GSPlayerState|Attributes")
	FGSOnGameplayAttributeValueChangedDelegate OnStaminaAttributeChanged;
	UPROPERTY(BlueprintAssignable, Category = "GASShooter|GSPlayerState|Attributes")
	FGSOnGameplayAttributeValueChangedDelegate OnMaxStaminaAttributeChanged;
	UPROPERTY(BlueprintAssignable, Category = "GASShooter|GSPlayerState|Attributes")
	FGSOnGameplayAttributeValueChangedDelegate OnStaminaRegenRateAttributeChanged;
	UPROPERTY(BlueprintAssignable, Category = "GASShooter|GSPlayerState|Attributes")
	FGSOnGameplayAttributeValueChangedDelegate OnXPAttributeChanged;
	UPROPERTY(BlueprintAssignable, Category = "GASShooter|GSPlayerState|Attributes")
	FGSOnGameplayAttributeValueChangedDelegate OnGoldAttributeChanged;
	UPROPERTY(BlueprintAssignable, Category = "GASShooter|GSPlayerState|Attributes")
	FGSOnGameplayAttributeValueChangedDelegate OnCharacterLevelAttributeChanged;
	*/

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	// Attribute changed callbacks
	virtual void HealthChanged(const FOnAttributeChangeData& Data);
	/*
	virtual void MaxHealthChanged(const FOnAttributeChangeData& Data);
	virtual void HealthRegenRateChanged(const FOnAttributeChangeData& Data);
	virtual void ManaChanged(const FOnAttributeChangeData& Data);
	virtual void MaxManaChanged(const FOnAttributeChangeData& Data);
	virtual void ManaRegenRateChanged(const FOnAttributeChangeData& Data);
	virtual void StaminaChanged(const FOnAttributeChangeData& Data);
	virtual void MaxStaminaChanged(const FOnAttributeChangeData& Data);
	virtual void StaminaRegenRateChanged(const FOnAttributeChangeData& Data);
	virtual void XPChanged(const FOnAttributeChangeData& Data);
	virtual void GoldChanged(const FOnAttributeChangeData& Data);
	virtual void CharacterLevelChanged(const FOnAttributeChangeData& Data);
	*/
};
