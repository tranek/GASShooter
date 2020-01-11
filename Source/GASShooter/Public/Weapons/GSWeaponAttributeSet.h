// Copyright 2020 Dan Kestranek.

#pragma once

#include "CoreMinimal.h"
#include "AttributeSet.h"
#include "AbilitySystemComponent.h"
#include "GSWeaponAttributeSet.generated.h"

// Uses macros from AttributeSet.h
#define ATTRIBUTE_ACCESSORS(ClassName, PropertyName) \
	GAMEPLAYATTRIBUTE_PROPERTY_GETTER(ClassName, PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_GETTER(PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_SETTER(PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_INITTER(PropertyName)


//TODO Delete this if we don't need it
/*
#define GSGAMEPLAYATTRIBUTE_REPNOTIFY(C, P) \
{ \
	static UProperty* ThisProperty = FindFieldChecked<UProperty>(C::StaticClass(), GET_MEMBER_NAME_CHECKED(C, P)); \
	if (GetOwningAbilitySystemComponent()) { GetOwningAbilitySystemComponent()->SetBaseAttributeValueFromReplication(P, FGameplayAttribute(ThisProperty)); }\
}
*/

/**
 * AttributeSet for weapons. Depending on your weapons, you may want to make this more minimal and subclass it to include
 * specific Attributes for types of weapons. For this sample project, we're making it very general to cover all our weapons
 * since we don't have many.
 */
UCLASS()
class GASSHOOTER_API UGSWeaponAttributeSet : public UAttributeSet
{
	GENERATED_BODY()
	
public:
	UGSWeaponAttributeSet();

	UPROPERTY(BlueprintReadOnly, Category = "Ammo", ReplicatedUsing = OnRep_PrimaryClipAmmo)
	FGameplayAttributeData PrimaryClipAmmo;
	ATTRIBUTE_ACCESSORS(UGSWeaponAttributeSet, PrimaryClipAmmo)

	// We may want weapon upgrades that increase the clip size
	UPROPERTY(BlueprintReadOnly, Category = "Ammo", ReplicatedUsing = OnRep_PrimaryMaxClipAmmo)
	FGameplayAttributeData PrimaryMaxClipAmmo;
	ATTRIBUTE_ACCESSORS(UGSWeaponAttributeSet, PrimaryMaxClipAmmo)

	UPROPERTY(BlueprintReadOnly, Category = "Ammo", ReplicatedUsing = OnRep_PrimaryReserveAmmo)
	FGameplayAttributeData PrimaryReserveAmmo;
	ATTRIBUTE_ACCESSORS(UGSWeaponAttributeSet, PrimaryReserveAmmo)

	// We may want backpack upgrades that increases maximum ammo that we can carry
	UPROPERTY(BlueprintReadOnly, Category = "Ammo", ReplicatedUsing = OnRep_PrimaryMaxAmmo)
	FGameplayAttributeData PrimaryMaxAmmo;
	ATTRIBUTE_ACCESSORS(UGSWeaponAttributeSet, PrimaryMaxAmmo)

	// Damage caused by the primary weapon fire
	UPROPERTY(BlueprintReadOnly, Category = "Damage", ReplicatedUsing = OnRep_PrimaryDamage)
	FGameplayAttributeData PrimaryDamage;
	ATTRIBUTE_ACCESSORS(UGSWeaponAttributeSet, PrimaryDamage)

	// Secondary ammo could be something like rifle grenades (I don't have those in this sample project
	// because I don't have animations / assets)
	UPROPERTY(BlueprintReadOnly, Category = "Ammo", ReplicatedUsing = OnRep_SecondaryClipAmmo)
	FGameplayAttributeData SecondaryClipAmmo;
	ATTRIBUTE_ACCESSORS(UGSWeaponAttributeSet, SecondaryClipAmmo)

	// We may want weapon upgrades that increase the clip size
	UPROPERTY(BlueprintReadOnly, Category = "Ammo", ReplicatedUsing = OnRep_SecondaryMaxClipAmmo)
	FGameplayAttributeData SecondaryMaxClipAmmo;
	ATTRIBUTE_ACCESSORS(UGSWeaponAttributeSet, SecondaryMaxClipAmmo)

	UPROPERTY(BlueprintReadOnly, Category = "Ammo", ReplicatedUsing = OnRep_SecondaryReserveAmmo)
	FGameplayAttributeData SecondaryReserveAmmo;
	ATTRIBUTE_ACCESSORS(UGSWeaponAttributeSet, SecondaryReserveAmmo)

	// We may want backpack upgrades that increases maximum ammo that we can carry
	UPROPERTY(BlueprintReadOnly, Category = "Ammo", ReplicatedUsing = OnRep_SecondaryMaxAmmo)
	FGameplayAttributeData SecondaryMaxAmmo;
	ATTRIBUTE_ACCESSORS(UGSWeaponAttributeSet, SecondaryMaxAmmo)

	// Damage caused by the secondary weapon fire / secondary ammo
	UPROPERTY(BlueprintReadOnly, Category = "Damage", ReplicatedUsing = OnRep_SecondaryDamage)
	FGameplayAttributeData SecondaryDamage;
	ATTRIBUTE_ACCESSORS(UGSWeaponAttributeSet, SecondaryDamage)

	virtual void PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue) override;
	virtual void PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

protected:
	UFUNCTION()
	virtual void OnRep_PrimaryClipAmmo();

	UFUNCTION()
	virtual void OnRep_PrimaryMaxClipAmmo();

	UFUNCTION()
	virtual void OnRep_PrimaryReserveAmmo();

	UFUNCTION()
	virtual void OnRep_PrimaryMaxAmmo();

	UFUNCTION()
	virtual void OnRep_PrimaryDamage();

	UFUNCTION()
	virtual void OnRep_SecondaryClipAmmo();

	UFUNCTION()
	virtual void OnRep_SecondaryMaxClipAmmo();

	UFUNCTION()
	virtual void OnRep_SecondaryReserveAmmo();

	UFUNCTION()
	virtual void OnRep_SecondaryMaxAmmo();

	UFUNCTION()
	virtual void OnRep_SecondaryDamage();
};
