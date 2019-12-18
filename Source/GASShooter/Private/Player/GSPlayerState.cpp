// Copyright 2019 Dan Kestranek.


#include "Player/GSPlayerState.h"
#include "Characters/Abilities/AttributeSets/GSAttributeSetBase.h"
#include "Characters/Abilities/GSAbilitySystemComponent.h"

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

		// Tag change callbacks
	}
}