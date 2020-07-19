// Copyright 2020 Dan Kestranek.


#include "Player/GSPlayerState.h"
#include "Characters/Abilities/AttributeSets/GSAmmoAttributeSet.h"
#include "Characters/Abilities/AttributeSets/GSAttributeSetBase.h"
#include "Characters/Abilities/GSAbilitySystemComponent.h"
#include "Characters/Abilities/GSAbilitySystemGlobals.h"
#include "Characters/Heroes/GSHeroCharacter.h"
#include "Player/GSPlayerController.h"
#include "UI/GSFloatingStatusBarWidget.h"
#include "UI/GSHUDWidget.h"
#include "Weapons/GSWeapon.h"

AGSPlayerState::AGSPlayerState()
{
	// Create ability system component, and set it to be explicitly replicated
	AbilitySystemComponent = CreateDefaultSubobject<UGSAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
	AbilitySystemComponent->SetIsReplicated(true);

	// Mixed mode means we only are replicated the GEs to ourself, not the GEs to simulated proxies. If another GDPlayerState (Hero) receives a GE,
	// we won't be told about it by the Server. Attributes, GameplayTags, and GameplayCues will still replicate to us.
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Mixed);

	// Create the attribute set, this replicates by default
	// Adding it as a subobject of the owning actor of an AbilitySystemComponent
	// automatically registers the AttributeSet with the AbilitySystemComponent
	AttributeSetBase = CreateDefaultSubobject<UGSAttributeSetBase>(TEXT("AttributeSetBase"));

	AmmoAttributeSet = CreateDefaultSubobject<UGSAmmoAttributeSet>(TEXT("AmmoAttributeSet"));

	// Set PlayerState's NetUpdateFrequency to the same as the Character.
	// Default is very low for PlayerStates and introduces perceived lag in the ability system.
	// 100 is probably way too high for a shipping game, you can adjust to fit your needs.
	NetUpdateFrequency = 100.0f;

	DeadTag = FGameplayTag::RequestGameplayTag("State.Dead");
	KnockedDownTag = FGameplayTag::RequestGameplayTag("State.KnockedDown");
}

UAbilitySystemComponent* AGSPlayerState::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

UGSAttributeSetBase* AGSPlayerState::GetAttributeSetBase() const
{
	return AttributeSetBase;
}

UGSAmmoAttributeSet* AGSPlayerState::GetAmmoAttributeSet() const
{
	return AmmoAttributeSet;
}

bool AGSPlayerState::IsAlive() const
{
	return GetHealth() > 0.0f;
}

void AGSPlayerState::ShowAbilityConfirmPrompt(bool bShowPrompt)
{
	AGSPlayerController* PC = Cast<AGSPlayerController>(GetOwner());
	if (PC)
	{
		UGSHUDWidget* HUD = PC->GetGSHUD();
		if (HUD)
		{
			HUD->ShowAbilityConfirmPrompt(bShowPrompt);
		}
	}
}

void AGSPlayerState::ShowInteractionPrompt(float InteractionDuration)
{
	AGSPlayerController* PC = Cast<AGSPlayerController>(GetOwner());
	if (PC)
	{
		UGSHUDWidget* HUD = PC->GetGSHUD();
		if (HUD)
		{
			HUD->ShowInteractionPrompt(InteractionDuration);
		}
	}
}

void AGSPlayerState::HideInteractionPrompt()
{
	AGSPlayerController* PC = Cast<AGSPlayerController>(GetOwner());
	if (PC)
	{
		UGSHUDWidget* HUD = PC->GetGSHUD();
		if (HUD)
		{
			HUD->HideInteractionPrompt();
		}
	}
}

void AGSPlayerState::StartInteractionTimer(float InteractionDuration)
{
	AGSPlayerController* PC = Cast<AGSPlayerController>(GetOwner());
	if (PC)
	{
		UGSHUDWidget* HUD = PC->GetGSHUD();
		if (HUD)
		{
			HUD->StartInteractionTimer(InteractionDuration);
		}
	}
}

void AGSPlayerState::StopInteractionTimer()
{
	AGSPlayerController* PC = Cast<AGSPlayerController>(GetOwner());
	if (PC)
	{
		UGSHUDWidget* HUD = PC->GetGSHUD();
		if (HUD)
		{
			HUD->StopInteractionTimer();
		}
	}
}

float AGSPlayerState::GetHealth() const
{
	return AttributeSetBase->GetHealth();
}

float AGSPlayerState::GetMaxHealth() const
{
	return AttributeSetBase->GetMaxHealth();
}

float AGSPlayerState::GetHealthRegenRate() const
{
	return AttributeSetBase->GetHealthRegenRate();
}

float AGSPlayerState::GetMana() const
{
	return AttributeSetBase->GetMana();
}

float AGSPlayerState::GetMaxMana() const
{
	return AttributeSetBase->GetMaxMana();
}

float AGSPlayerState::GetManaRegenRate() const
{
	return AttributeSetBase->GetManaRegenRate();
}

float AGSPlayerState::GetStamina() const
{
	return AttributeSetBase->GetStamina();
}

float AGSPlayerState::GetMaxStamina() const
{
	return AttributeSetBase->GetMaxStamina();
}

float AGSPlayerState::GetStaminaRegenRate() const
{
	return AttributeSetBase->GetStaminaRegenRate();
}

float AGSPlayerState::GetShield() const
{
	return AttributeSetBase->GetShield();
}

float AGSPlayerState::GetMaxShield() const
{
	return AttributeSetBase->GetMaxShield();
}

float AGSPlayerState::GetShieldRegenRate() const
{
	return AttributeSetBase->GetShieldRegenRate();
}

float AGSPlayerState::GetArmor() const
{
	return AttributeSetBase->GetArmor();
}

float AGSPlayerState::GetMoveSpeed() const
{
	return AttributeSetBase->GetMoveSpeed();
}

int32 AGSPlayerState::GetCharacterLevel() const
{
	return AttributeSetBase->GetCharacterLevel();
}

int32 AGSPlayerState::GetXP() const
{
	return AttributeSetBase->GetXP();
}

int32 AGSPlayerState::GetXPBounty() const
{
	return AttributeSetBase->GetXPBounty();
}

int32 AGSPlayerState::GetGold() const
{
	return AttributeSetBase->GetGold();
}

int32 AGSPlayerState::GetGoldBounty() const
{
	return AttributeSetBase->GetGoldBounty();
}

int32 AGSPlayerState::GetPrimaryClipAmmo() const
{
	AGSHeroCharacter* Hero = GetPawn<AGSHeroCharacter>();
	if (Hero)
	{
		return Hero->GetPrimaryClipAmmo();
	}

	return 0;
}

int32 AGSPlayerState::GetPrimaryReserveAmmo() const
{
	AGSHeroCharacter* Hero = GetPawn<AGSHeroCharacter>();
	if (Hero && Hero->GetCurrentWeapon() && AmmoAttributeSet)
	{
		FGameplayAttribute Attribute = AmmoAttributeSet->GetReserveAmmoAttributeFromTag(Hero->GetCurrentWeapon()->PrimaryAmmoType);
		if (Attribute.IsValid())
		{
			return AbilitySystemComponent->GetNumericAttribute(Attribute);
		}
	}

	return 0;
}

void AGSPlayerState::BeginPlay()
{
	Super::BeginPlay();

	if (AbilitySystemComponent)
	{
		// Attribute change callbacks
		HealthChangedDelegateHandle = AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(AttributeSetBase->GetHealthAttribute()).AddUObject(this, &AGSPlayerState::HealthChanged);

		// Tag change callbacks
		AbilitySystemComponent->RegisterGameplayTagEvent(KnockedDownTag, EGameplayTagEventType::NewOrRemoved).AddUObject(this, &AGSPlayerState::KnockDownTagChanged);
	}
}

void AGSPlayerState::HealthChanged(const FOnAttributeChangeData& Data)
{
	// Check for and handle knockdown and death
	AGSHeroCharacter* Hero = Cast<AGSHeroCharacter>(GetPawn());
	if (IsValid(Hero) && !IsAlive() && !AbilitySystemComponent->HasMatchingGameplayTag(DeadTag))
	{
		if (Hero)
		{
			if (!AbilitySystemComponent->HasMatchingGameplayTag(KnockedDownTag))
			{
				Hero->KnockDown();
			}
			else
			{
				Hero->FinishDying();
			}
		}
	}
}

void AGSPlayerState::KnockDownTagChanged(const FGameplayTag CallbackTag, int32 NewCount)
{
	AGSHeroCharacter* Hero = Cast<AGSHeroCharacter>(GetPawn());
	if (!IsValid(Hero))
	{
		return;
	}

	if (NewCount > 0)
	{
		Hero->PlayKnockDownEffects();
	}
	else if (NewCount < 1 && !AbilitySystemComponent->HasMatchingGameplayTag(DeadTag))
	{
		Hero->PlayReviveEffects();
	}
}
