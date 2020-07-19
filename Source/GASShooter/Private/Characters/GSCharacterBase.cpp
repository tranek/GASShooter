// Copyright 2020 Dan Kestranek.


#include "Characters/GSCharacterBase.h"
#include "Characters/Abilities/AttributeSets/GSAttributeSetBase.h"
#include "Characters/Abilities/GSAbilitySystemComponent.h"
#include "Characters/Abilities/GSAbilitySystemGlobals.h"
#include "Characters/Abilities/GSGameplayAbility.h"
#include "Characters/GSCharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"
#include "UI/GSDamageTextWidgetComponent.h"

// Sets default values
AGSCharacterBase::AGSCharacterBase(const class FObjectInitializer& ObjectInitializer) :
	Super(ObjectInitializer.SetDefaultSubobjectClass<UGSCharacterMovementComponent>(ACharacter::CharacterMovementComponentName))
{
	PrimaryActorTick.bCanEverTick = false;

	GetCapsuleComponent()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Overlap);
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);

	bAlwaysRelevant = true;

	// Cache tags
	DeadTag = FGameplayTag::RequestGameplayTag("State.Dead");
	EffectRemoveOnDeathTag = FGameplayTag::RequestGameplayTag("Effect.RemoveOnDeath");

	// Hardcoding to avoid having to manually set for every Blueprint child class
	DamageNumberClass = StaticLoadClass(UObject::StaticClass(), nullptr, TEXT("/Game/GASShooter/UI/WC_DamageText.WC_DamageText_C"));
	if (!DamageNumberClass)
	{
		UE_LOG(LogTemp, Error, TEXT("%s() Failed to find DamageNumberClass. If it was moved, please update the reference location in C++."), *FString(__FUNCTION__));
	}
}

UAbilitySystemComponent* AGSCharacterBase::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

bool AGSCharacterBase::IsAlive() const
{
	return GetHealth() > 0.0f;
}

int32 AGSCharacterBase::GetAbilityLevel(EGSAbilityInputID AbilityID) const
{
	//TODO
	return 1;
}

void AGSCharacterBase::RemoveCharacterAbilities()
{
	if (GetLocalRole() != ROLE_Authority || !IsValid(AbilitySystemComponent) || !AbilitySystemComponent->bCharacterAbilitiesGiven)
	{
		return;
	}

	// Remove any abilities added from a previous call. This checks to make sure the ability is in the startup 'CharacterAbilities' array.
	TArray<FGameplayAbilitySpecHandle> AbilitiesToRemove;
	for (const FGameplayAbilitySpec& Spec : AbilitySystemComponent->GetActivatableAbilities())
	{
		if ((Spec.SourceObject == this) && CharacterAbilities.Contains(Spec.Ability->GetClass()))
		{
			AbilitiesToRemove.Add(Spec.Handle);
		}
	}

	// Do in two passes so the removal happens after we have the full list
	for (int32 i = 0; i < AbilitiesToRemove.Num(); i++)
	{
		AbilitySystemComponent->ClearAbility(AbilitiesToRemove[i]);
	}

	AbilitySystemComponent->bCharacterAbilitiesGiven = false;
}

void AGSCharacterBase::Die()
{
	// Only runs on Server
	RemoveCharacterAbilities();

	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GetCharacterMovement()->GravityScale = 0;
	GetCharacterMovement()->Velocity = FVector(0);

	OnCharacterDied.Broadcast(this);

	if (IsValid(AbilitySystemComponent))
	{
		AbilitySystemComponent->CancelAllAbilities();

		FGameplayTagContainer EffectTagsToRemove;
		EffectTagsToRemove.AddTag(EffectRemoveOnDeathTag);
		int32 NumEffectsRemoved = AbilitySystemComponent->RemoveActiveEffectsWithTags(EffectTagsToRemove);

		AbilitySystemComponent->AddLooseGameplayTag(DeadTag);
	}

	//TODO replace with a locally executed GameplayCue
	if (DeathSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, DeathSound, GetActorLocation());
	}

	if (DeathMontage)
	{
		PlayAnimMontage(DeathMontage);
	}
	else
	{
		FinishDying();
	}
}

void AGSCharacterBase::FinishDying()
{
	Destroy();
}

void AGSCharacterBase::AddDamageNumber(float Damage, FGameplayTagContainer DamageNumberTags)
{
	DamageNumberQueue.Add(FGSDamageNumber(Damage, DamageNumberTags));

	if (!GetWorldTimerManager().IsTimerActive(DamageNumberTimer))
	{
		GetWorldTimerManager().SetTimer(DamageNumberTimer, this, &AGSCharacterBase::ShowDamageNumber, 0.1, true, 0.0f);
	}
}

int32 AGSCharacterBase::GetCharacterLevel() const
{
	//TODO
	return 1;
}

float AGSCharacterBase::GetHealth() const
{
	if (IsValid(AttributeSetBase))
	{
		return AttributeSetBase->GetHealth();
	}

	return 0.0f;
}

float AGSCharacterBase::GetMaxHealth() const
{
	if (IsValid(AttributeSetBase))
	{
		return AttributeSetBase->GetMaxHealth();
	}
	
	return 0.0f;
}

float AGSCharacterBase::GetMana() const
{
	if (IsValid(AttributeSetBase))
	{
		return AttributeSetBase->GetMana();
	}

	return 0.0f;
}

float AGSCharacterBase::GetMaxMana() const
{
	if (IsValid(AttributeSetBase))
	{
		return AttributeSetBase->GetMaxMana();
	}

	return 0.0f;
}

float AGSCharacterBase::GetStamina() const
{
	if (IsValid(AttributeSetBase))
	{
		return AttributeSetBase->GetStamina();
	}

	return 0.0f;
}

float AGSCharacterBase::GetMaxStamina() const
{
	if (IsValid(AttributeSetBase))
	{
		return AttributeSetBase->GetMaxStamina();
	}

	return 0.0f;
}

float AGSCharacterBase::GetShield() const
{
	if (IsValid(AttributeSetBase))
	{
		return AttributeSetBase->GetShield();
	}

	return 0.0f;
}

float AGSCharacterBase::GetMaxShield() const
{
	if (IsValid(AttributeSetBase))
	{
		return AttributeSetBase->GetMaxShield();
	}

	return 0.0f;
}

float AGSCharacterBase::GetMoveSpeed() const
{
	if (IsValid(AttributeSetBase))
	{
		return AttributeSetBase->GetMoveSpeed();
	}

	return 0.0f;
}

float AGSCharacterBase::GetMoveSpeedBaseValue() const
{
	if (IsValid(AttributeSetBase))
	{
		return AttributeSetBase->GetMoveSpeedAttribute().GetGameplayAttributeData(AttributeSetBase)->GetBaseValue();
	}

	return 0.0f;
}

// Called when the game starts or when spawned
void AGSCharacterBase::BeginPlay()
{
	Super::BeginPlay();
}

void AGSCharacterBase::AddCharacterAbilities()
{
	// Grant abilities, but only on the server	
	if (GetLocalRole() != ROLE_Authority || !IsValid(AbilitySystemComponent) || AbilitySystemComponent->bCharacterAbilitiesGiven)
	{
		return;
	}

	for (TSubclassOf<UGSGameplayAbility>& StartupAbility : CharacterAbilities)
	{
		AbilitySystemComponent->GiveAbility(
			FGameplayAbilitySpec(StartupAbility, GetAbilityLevel(StartupAbility.GetDefaultObject()->AbilityID), static_cast<int32>(StartupAbility.GetDefaultObject()->AbilityInputID), this));
	}

	AbilitySystemComponent->bCharacterAbilitiesGiven = true;
}

void AGSCharacterBase::InitializeAttributes()
{
	if (!IsValid(AbilitySystemComponent))
	{
		return;
	}

	if (!DefaultAttributes)
	{
		UE_LOG(LogTemp, Error, TEXT("%s() Missing DefaultAttributes for %s. Please fill in the character's Blueprint."), *FString(__FUNCTION__), *GetName());
		return;
	}

	// Can run on Server and Client
	FGameplayEffectContextHandle EffectContext = AbilitySystemComponent->MakeEffectContext();
	EffectContext.AddSourceObject(this);

	FGameplayEffectSpecHandle NewHandle = AbilitySystemComponent->MakeOutgoingSpec(DefaultAttributes, GetCharacterLevel(), EffectContext);
	if (NewHandle.IsValid())
	{
		FActiveGameplayEffectHandle ActiveGEHandle = AbilitySystemComponent->ApplyGameplayEffectSpecToSelf(*NewHandle.Data.Get());
	}
}

void AGSCharacterBase::AddStartupEffects()
{
	if (GetLocalRole() != ROLE_Authority || !IsValid(AbilitySystemComponent) || AbilitySystemComponent->bStartupEffectsApplied)
	{
		return;
	}

	FGameplayEffectContextHandle EffectContext = AbilitySystemComponent->MakeEffectContext();
	EffectContext.AddSourceObject(this);

	for (TSubclassOf<UGameplayEffect> GameplayEffect : StartupEffects)
	{
		FGameplayEffectSpecHandle NewHandle = AbilitySystemComponent->MakeOutgoingSpec(GameplayEffect, GetCharacterLevel(), EffectContext);
		if (NewHandle.IsValid())
		{
			FActiveGameplayEffectHandle ActiveGEHandle = AbilitySystemComponent->ApplyGameplayEffectSpecToTarget(*NewHandle.Data.Get(), AbilitySystemComponent);
		}
	}

	AbilitySystemComponent->bStartupEffectsApplied = true;
}

void AGSCharacterBase::ShowDamageNumber()
{
	if (DamageNumberQueue.Num() > 0 && IsValid(this))
	{
		UGSDamageTextWidgetComponent* DamageText = NewObject<UGSDamageTextWidgetComponent>(this, DamageNumberClass);
		DamageText->RegisterComponent();
		DamageText->AttachToComponent(GetRootComponent(), FAttachmentTransformRules::KeepRelativeTransform);
		DamageText->SetDamageText(DamageNumberQueue[0].DamageAmount, DamageNumberQueue[0].Tags);

		if (DamageNumberQueue.Num() < 1)
		{
			GetWorldTimerManager().ClearTimer(DamageNumberTimer);
		}

		DamageNumberQueue.RemoveAt(0);
	}
}

void AGSCharacterBase::SetHealth(float Health)
{
	if (IsValid(AttributeSetBase))
	{
		AttributeSetBase->SetHealth(Health);
	}
}

void AGSCharacterBase::SetMana(float Mana)
{
	if (IsValid(AttributeSetBase))
	{
		AttributeSetBase->SetMana(Mana);
	}
}

void AGSCharacterBase::SetStamina(float Stamina)
{
	if (IsValid(AttributeSetBase))
	{
		AttributeSetBase->SetStamina(Stamina);
	}
}

void AGSCharacterBase::SetShield(float Shield)
{
	if (IsValid(AttributeSetBase))
	{
		AttributeSetBase->SetShield(Shield);
	}
}
