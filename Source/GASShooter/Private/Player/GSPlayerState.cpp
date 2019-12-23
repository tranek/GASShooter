// Copyright 2019 Dan Kestranek.


#include "Player/GSPlayerState.h"
#include "Characters/Abilities/AttributeSets/GSAttributeSetBase.h"
#include "Characters/Abilities/GSAbilitySystemComponent.h"
#include "Characters/Heroes/GSHeroCharacter.h"
#include "Player/GSPlayerController.h"
#include "UI/GSFloatingStatusBarWidget.h"
#include "UI/GSHUDWidget.h"

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

	// Set PlayerState's NetUpdateFrequency to the same as the Character.
	// Default is very low for PlayerStates and introduces perceived lag in the ability system.
	// 100 is probably way too high for a shipping game, you can adjust to fit your needs.
	NetUpdateFrequency = 100.0f;
}

UAbilitySystemComponent* AGSPlayerState::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

UGSAttributeSetBase* AGSPlayerState::GetAttributeSetBase() const
{
	return AttributeSetBase;
}

bool AGSPlayerState::IsAlive() const
{
	return GetHealth() > 0.0f;
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

void AGSPlayerState::BeginPlay()
{
	Super::BeginPlay();

	if (AbilitySystemComponent)
	{
		// Attribute change callbacks
		HealthChangedDelegateHandle = AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(AttributeSetBase->GetHealthAttribute()).AddUObject(this, &AGSPlayerState::HealthChanged);

		/*
		MaxHealthChangedDelegateHandle = AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(AttributeSetBase->GetMaxHealthAttribute()).AddUObject(this, &AGSPlayerState::MaxHealthChanged);
		HealthRegenRateChangedDelegateHandle = AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(AttributeSetBase->GetHealthRegenRateAttribute()).AddUObject(this, &AGSPlayerState::HealthRegenRateChanged);
		ManaChangedDelegateHandle = AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(AttributeSetBase->GetManaAttribute()).AddUObject(this, &AGSPlayerState::ManaChanged);
		MaxManaChangedDelegateHandle = AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(AttributeSetBase->GetMaxManaAttribute()).AddUObject(this, &AGSPlayerState::MaxManaChanged);
		ManaRegenRateChangedDelegateHandle = AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(AttributeSetBase->GetManaRegenRateAttribute()).AddUObject(this, &AGSPlayerState::ManaRegenRateChanged);
		StaminaChangedDelegateHandle = AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(AttributeSetBase->GetStaminaAttribute()).AddUObject(this, &AGSPlayerState::StaminaChanged);
		MaxStaminaChangedDelegateHandle = AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(AttributeSetBase->GetMaxStaminaAttribute()).AddUObject(this, &AGSPlayerState::MaxStaminaChanged);
		StaminaRegenRateChangedDelegateHandle = AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(AttributeSetBase->GetStaminaRegenRateAttribute()).AddUObject(this, &AGSPlayerState::StaminaRegenRateChanged);
		XPChangedDelegateHandle = AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(AttributeSetBase->GetXPAttribute()).AddUObject(this, &AGSPlayerState::XPChanged);
		GoldChangedDelegateHandle = AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(AttributeSetBase->GetGoldAttribute()).AddUObject(this, &AGSPlayerState::GoldChanged);
		CharacterLevelChangedDelegateHandle = AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(AttributeSetBase->GetCharacterLevelAttribute()).AddUObject(this, &AGSPlayerState::CharacterLevelChanged);
		*/

		// Tag change callbacks
	}
}

void AGSPlayerState::HealthChanged(const FOnAttributeChangeData& Data)
{
	//float Health = Data.NewValue;

	//OnHealthAttributeChanged.Broadcast(Data.Attribute, Data.NewValue, Data.OldValue);

	//TODO
	// If the player died, handle death
	/*
	if (!IsAlive() && !AbilitySystemComponent->HasMatchingGameplayTag(DeadTag))
	{
		if (Hero)
		{
			Hero->Die();
		}
	}
	*/
}

/*
void AGSPlayerState::MaxHealthChanged(const FOnAttributeChangeData& Data)
{
	OnMaxHealthAttributeChanged.Broadcast(Data.Attribute, Data.NewValue, Data.OldValue);
}

void AGSPlayerState::HealthRegenRateChanged(const FOnAttributeChangeData& Data)
{
	OnHealthRegenRateAttributeChanged.Broadcast(Data.Attribute, Data.NewValue, Data.OldValue);
}

void AGSPlayerState::ManaChanged(const FOnAttributeChangeData& Data)
{
	OnManaAttributeChanged.Broadcast(Data.Attribute, Data.NewValue, Data.OldValue);
}

void AGSPlayerState::MaxManaChanged(const FOnAttributeChangeData& Data)
{
	OnMaxManaAttributeChanged.Broadcast(Data.Attribute, Data.NewValue, Data.OldValue);
}

void AGSPlayerState::ManaRegenRateChanged(const FOnAttributeChangeData& Data)
{
	OnManaRegenRateAttributeChanged.Broadcast(Data.Attribute, Data.NewValue, Data.OldValue);
}

void AGSPlayerState::StaminaChanged(const FOnAttributeChangeData& Data)
{
	OnStaminaAttributeChanged.Broadcast(Data.Attribute, Data.NewValue, Data.OldValue);
}

void AGSPlayerState::MaxStaminaChanged(const FOnAttributeChangeData& Data)
{
	OnMaxStaminaAttributeChanged.Broadcast(Data.Attribute, Data.NewValue, Data.OldValue);
}

void AGSPlayerState::StaminaRegenRateChanged(const FOnAttributeChangeData& Data)
{
	OnStaminaRegenRateAttributeChanged.Broadcast(Data.Attribute, Data.NewValue, Data.OldValue);
}

void AGSPlayerState::XPChanged(const FOnAttributeChangeData& Data)
{
	OnXPAttributeChanged.Broadcast(Data.Attribute, Data.NewValue, Data.OldValue);
}

void AGSPlayerState::GoldChanged(const FOnAttributeChangeData& Data)
{
	OnGoldAttributeChanged.Broadcast(Data.Attribute, Data.NewValue, Data.OldValue);
}

void AGSPlayerState::CharacterLevelChanged(const FOnAttributeChangeData& Data)
{
	OnCharacterLevelAttributeChanged.Broadcast(Data.Attribute, Data.NewValue, Data.OldValue);
}
*/
