// Copyright 2020 Dan Kestranek.


#include "Weapons/GSWeaponAttributeSet.h"
#include "GameplayEffect.h"
#include "GameplayEffectExtension.h"
#include "Net/UnrealNetwork.h"

UGSWeaponAttributeSet::UGSWeaponAttributeSet()
{
}

void UGSWeaponAttributeSet::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
	Super::PreAttributeChange(Attribute, NewValue);

	if (Attribute == GetPrimaryMaxAmmoAttribute())
	{
		// If our primary max ammo drops, clamp primary reserve ammo
		SetPrimaryReserveAmmo(FMath::Clamp<float>(GetPrimaryReserveAmmo(), 0, GetPrimaryMaxAmmo() - (GetPrimaryMaxClipAmmo() - GetPrimaryClipAmmo())));
	}
	else if (Attribute == GetSecondaryMaxAmmoAttribute())
	{
		// If our secondary max ammo drops, clamp secondary reserve ammo
		SetSecondaryReserveAmmo(FMath::Clamp<float>(GetSecondaryReserveAmmo(), 0, GetSecondaryMaxAmmo() - (GetSecondaryMaxClipAmmo() - GetSecondaryClipAmmo())));
	}
}

void UGSWeaponAttributeSet::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)
{
	Super::PostGameplayEffectExecute(Data);

	if (Data.EvaluatedData.Attribute == GetPrimaryClipAmmoAttribute())
	{
		//TODO Will we ever get anything that pushes our clip ammo above max?
		// If so we need to put remainder into Reserve Ammo.
		// But I think only reload will ever add to clip ammo and that should intelligently
		// only subtract just enough from reserve ammo.
		SetPrimaryClipAmmo(FMath::Clamp<float>(GetPrimaryClipAmmo(), 0, GetPrimaryMaxClipAmmo()));
	}
	else if (Data.EvaluatedData.Attribute == GetPrimaryReserveAmmoAttribute())
	{
		SetPrimaryReserveAmmo(FMath::Clamp<float>(GetPrimaryReserveAmmo(), 0, GetPrimaryMaxAmmo() - (GetPrimaryMaxClipAmmo() - GetPrimaryClipAmmo())));
	}
	else if (Data.EvaluatedData.Attribute == GetSecondaryClipAmmoAttribute())
	{
		SetSecondaryClipAmmo(FMath::Clamp<float>(GetSecondaryClipAmmo(), 0, GetSecondaryMaxClipAmmo()));
	}
	else if (Data.EvaluatedData.Attribute == GetSecondaryReserveAmmoAttribute())
	{
		SetSecondaryReserveAmmo(FMath::Clamp<float>(GetSecondaryReserveAmmo(), 0, GetSecondaryMaxAmmo() - (GetSecondaryMaxClipAmmo() - GetSecondaryClipAmmo())));
	}
}

void UGSWeaponAttributeSet::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION_NOTIFY(UGSWeaponAttributeSet, PrimaryClipAmmo, COND_OwnerOnly, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UGSWeaponAttributeSet, PrimaryMaxClipAmmo, COND_OwnerOnly, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UGSWeaponAttributeSet, PrimaryReserveAmmo, COND_OwnerOnly, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UGSWeaponAttributeSet, PrimaryMaxAmmo, COND_OwnerOnly, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UGSWeaponAttributeSet, PrimaryDamage, COND_OwnerOnly, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UGSWeaponAttributeSet, SecondaryClipAmmo, COND_OwnerOnly, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UGSWeaponAttributeSet, SecondaryMaxClipAmmo, COND_OwnerOnly, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UGSWeaponAttributeSet, SecondaryReserveAmmo, COND_OwnerOnly, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UGSWeaponAttributeSet, SecondaryMaxAmmo, COND_OwnerOnly, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UGSWeaponAttributeSet, SecondaryDamage, COND_OwnerOnly, REPNOTIFY_Always);
}

void UGSWeaponAttributeSet::OnRep_PrimaryClipAmmo()
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UGSWeaponAttributeSet, PrimaryClipAmmo);
	//GSGAMEPLAYATTRIBUTE_REPNOTIFY(UGSWeaponAttributeSet, PrimaryClipAmmo);
}

void UGSWeaponAttributeSet::OnRep_PrimaryMaxClipAmmo()
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UGSWeaponAttributeSet, PrimaryMaxClipAmmo);
	//GSGAMEPLAYATTRIBUTE_REPNOTIFY(UGSWeaponAttributeSet, PrimaryMaxClipAmmo);
}

void UGSWeaponAttributeSet::OnRep_PrimaryReserveAmmo()
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UGSWeaponAttributeSet, PrimaryMaxClipAmmo);
	//GSGAMEPLAYATTRIBUTE_REPNOTIFY(UGSWeaponAttributeSet, PrimaryReserveAmmo);
}

void UGSWeaponAttributeSet::OnRep_PrimaryMaxAmmo()
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UGSWeaponAttributeSet, PrimaryMaxAmmo);
	//GSGAMEPLAYATTRIBUTE_REPNOTIFY(UGSWeaponAttributeSet, PrimaryMaxAmmo);
}

void UGSWeaponAttributeSet::OnRep_PrimaryDamage()
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UGSWeaponAttributeSet, PrimaryDamage);
	//GSGAMEPLAYATTRIBUTE_REPNOTIFY(UGSWeaponAttributeSet, PrimaryDamage);
}

void UGSWeaponAttributeSet::OnRep_SecondaryClipAmmo()
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UGSWeaponAttributeSet, SecondaryClipAmmo);
	//GSGAMEPLAYATTRIBUTE_REPNOTIFY(UGSWeaponAttributeSet, SecondaryClipAmmo);
}

void UGSWeaponAttributeSet::OnRep_SecondaryMaxClipAmmo()
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UGSWeaponAttributeSet, SecondaryMaxClipAmmo);
	//GSGAMEPLAYATTRIBUTE_REPNOTIFY(UGSWeaponAttributeSet, SecondaryMaxClipAmmo);
}

void UGSWeaponAttributeSet::OnRep_SecondaryReserveAmmo()
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UGSWeaponAttributeSet, SecondaryReserveAmmo);
	//GSGAMEPLAYATTRIBUTE_REPNOTIFY(UGSWeaponAttributeSet, SecondaryReserveAmmo);
}

void UGSWeaponAttributeSet::OnRep_SecondaryMaxAmmo()
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UGSWeaponAttributeSet, SecondaryMaxAmmo);
	//GSGAMEPLAYATTRIBUTE_REPNOTIFY(UGSWeaponAttributeSet, SecondaryMaxAmmo);
}

void UGSWeaponAttributeSet::OnRep_SecondaryDamage()
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UGSWeaponAttributeSet, SecondaryDamage);
	//GSGAMEPLAYATTRIBUTE_REPNOTIFY(UGSWeaponAttributeSet, SecondaryDamage);
}
