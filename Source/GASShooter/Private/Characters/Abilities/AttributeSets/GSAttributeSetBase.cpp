// Copyright 2020 Dan Kestranek.


#include "Characters/Abilities/AttributeSets/GSAttributeSetBase.h"
#include "Characters/GSCharacterBase.h"
#include "GameplayEffect.h"
#include "GameplayEffectExtension.h"
#include "Net/UnrealNetwork.h"
#include "Player/GSPlayerController.h"

UGSAttributeSetBase::UGSAttributeSetBase()
{
	//TODO move this into GASShooter since multiple files look for these tags
	// Cache tags
	HitDirectionFrontTag = FGameplayTag::RequestGameplayTag(FName("Effect.HitReact.Front"));
	HitDirectionBackTag = FGameplayTag::RequestGameplayTag(FName("Effect.HitReact.Back"));
	HitDirectionRightTag = FGameplayTag::RequestGameplayTag(FName("Effect.HitReact.Right"));
	HitDirectionLeftTag = FGameplayTag::RequestGameplayTag(FName("Effect.HitReact.Left"));
	HeadShotTag = FGameplayTag::RequestGameplayTag(FName("Effect.Damage.HeadShot"));
}

void UGSAttributeSetBase::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
	// This is called whenever attributes change, so for max health/mana we want to scale the current totals to match
	Super::PreAttributeChange(Attribute, NewValue);

	// If a Max value changes, adjust current to keep Current % of Current to Max
	if (Attribute == GetMaxHealthAttribute()) // GetMaxHealthAttribute comes from the Macros defined at the top of the header
	{
		AdjustAttributeForMaxChange(Health, MaxHealth, NewValue, GetHealthAttribute());
	}
	else if (Attribute == GetMaxManaAttribute())
	{
		AdjustAttributeForMaxChange(Mana, MaxMana, NewValue, GetManaAttribute());
	}
	else if (Attribute == GetMaxStaminaAttribute())
	{
		AdjustAttributeForMaxChange(Stamina, MaxStamina, NewValue, GetStaminaAttribute());
	}
	else if (Attribute == GetMoveSpeedAttribute())
	{
		// Cannot slow less than 150 units/s and cannot boost more than 1000 units/s
		NewValue = FMath::Clamp<float>(NewValue, 150, 1000);
	}
}

void UGSAttributeSetBase::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)
{
	Super::PostGameplayEffectExecute(Data);

	FGameplayEffectContextHandle Context = Data.EffectSpec.GetContext();
	UAbilitySystemComponent* Source = Context.GetOriginalInstigatorAbilitySystemComponent();
	const FGameplayTagContainer& SourceTags = *Data.EffectSpec.CapturedSourceTags.GetAggregatedTags();
	FGameplayTagContainer SpecAssetTags;
	Data.EffectSpec.GetAllAssetTags(SpecAssetTags);

	// Get the Target actor, which should be our owner
	AActor* TargetActor = nullptr;
	AController* TargetController = nullptr;
	AGSCharacterBase* TargetCharacter = nullptr;
	if (Data.Target.AbilityActorInfo.IsValid() && Data.Target.AbilityActorInfo->AvatarActor.IsValid())
	{
		TargetActor = Data.Target.AbilityActorInfo->AvatarActor.Get();
		TargetController = Data.Target.AbilityActorInfo->PlayerController.Get();
		TargetCharacter = Cast<AGSCharacterBase>(TargetActor);
	}

	// Get the Source actor
	AActor* SourceActor = nullptr;
	AController* SourceController = nullptr;
	AGSCharacterBase* SourceCharacter = nullptr;
	if (Source && Source->AbilityActorInfo.IsValid() && Source->AbilityActorInfo->AvatarActor.IsValid())
	{
		SourceActor = Source->AbilityActorInfo->AvatarActor.Get();
		SourceController = Source->AbilityActorInfo->PlayerController.Get();
		if (SourceController == nullptr && SourceActor != nullptr)
		{
			if (APawn* Pawn = Cast<APawn>(SourceActor))
			{
				SourceController = Pawn->GetController();
			}
		}

		// Use the controller to find the source pawn
		if (SourceController)
		{
			SourceCharacter = Cast<AGSCharacterBase>(SourceController->GetPawn());
		}
		else
		{
			SourceCharacter = Cast<AGSCharacterBase>(SourceActor);
		}

		// Set the causer actor based on context if it's set
		if (Context.GetEffectCauser())
		{
			SourceActor = Context.GetEffectCauser();
		}
	}

	if (Data.EvaluatedData.Attribute == GetDamageAttribute())
	{
		// Store a local copy of the amount of damage done and clear the damage attribute
		const float LocalDamageDone = GetDamage();
		SetDamage(0.f);

		if (LocalDamageDone > 0.0f)
		{
			// If character was alive before damage is added, handle damage
			// This prevents damage being added to dead things and replaying death animations
			bool WasAlive = true;

			if (TargetCharacter)
			{
				WasAlive = TargetCharacter->IsAlive();
			}

			if (!TargetCharacter->IsAlive())
			{
				//UE_LOG(LogTemp, Warning, TEXT("%s() %s is NOT alive when receiving damage"), TEXT(__FUNCTION__), *TargetCharacter->GetName());
			}

			// Apply the damage to shield first if it exists
			const float OldShield = GetShield();
			float DamageAfterShield = LocalDamageDone - OldShield;
			if (OldShield > 0)
			{
				float NewShield = OldShield - LocalDamageDone;
				SetShield(FMath::Clamp<float>(NewShield, 0.0f, GetMaxShield()));
			}

			if (DamageAfterShield > 0)
			{
				// Apply the health change and then clamp it
				const float NewHealth = GetHealth() - DamageAfterShield;
				SetHealth(FMath::Clamp(NewHealth, 0.0f, GetMaxHealth()));
			}

			if (TargetCharacter && WasAlive)
			{
				// This is the log statement for damage received. Turned off for live games.
				//UE_LOG(LogTemp, Log, TEXT("%s() %s Damage Received: %f"), TEXT(__FUNCTION__), *GetOwningActor()->GetName(), LocalDamageDone);

				// Show damage number for the Source player unless it was self damage
				if (SourceActor != TargetActor)
				{
					AGSPlayerController* PC = Cast<AGSPlayerController>(SourceController);
					if (PC)
					{
						FGameplayTagContainer DamageNumberTags;

						if (Data.EffectSpec.DynamicAssetTags.HasTag(HeadShotTag))
						{
							DamageNumberTags.AddTagFast(HeadShotTag);
						}

						PC->ShowDamageNumber(LocalDamageDone, TargetCharacter, DamageNumberTags);
					}
				}

				if (!TargetCharacter->IsAlive())
				{
					// TargetCharacter was alive before this damage and now is not alive, give XP and Gold bounties to Source.
					// Don't give bounty to self.
					if (SourceController != TargetController)
					{
						// Create a dynamic instant Gameplay Effect to give the bounties
						UGameplayEffect* GEBounty = NewObject<UGameplayEffect>(GetTransientPackage(), FName(TEXT("Bounty")));
						GEBounty->DurationPolicy = EGameplayEffectDurationType::Instant;

						int32 Idx = GEBounty->Modifiers.Num();
						GEBounty->Modifiers.SetNum(Idx + 2);

						FGameplayModifierInfo& InfoXP = GEBounty->Modifiers[Idx];
						InfoXP.ModifierMagnitude = FScalableFloat(GetXPBounty());
						InfoXP.ModifierOp = EGameplayModOp::Additive;
						InfoXP.Attribute = UGSAttributeSetBase::GetXPAttribute();

						FGameplayModifierInfo& InfoGold = GEBounty->Modifiers[Idx + 1];
						InfoGold.ModifierMagnitude = FScalableFloat(GetGoldBounty());
						InfoGold.ModifierOp = EGameplayModOp::Additive;
						InfoGold.Attribute = UGSAttributeSetBase::GetGoldAttribute();

						Source->ApplyGameplayEffectToSelf(GEBounty, 1.0f, Source->MakeEffectContext());
					}
				}
			}
		}
	}// Damage
	else if (Data.EvaluatedData.Attribute == GetHealthAttribute())
	{
		// Handle other health changes.
		// Health loss should go through Damage.
		SetHealth(FMath::Clamp(GetHealth(), 0.0f, GetMaxHealth()));
	} // Health
	else if (Data.EvaluatedData.Attribute == GetManaAttribute())
	{
		// Handle mana changes.
		SetMana(FMath::Clamp(GetMana(), 0.0f, GetMaxMana()));
	} // Mana
	else if (Data.EvaluatedData.Attribute == GetStaminaAttribute())
	{
		// Handle stamina changes.
		SetStamina(FMath::Clamp(GetStamina(), 0.0f, GetMaxStamina()));
	}
	else if (Data.EvaluatedData.Attribute == GetShieldAttribute())
	{
		// Handle shield changes.
		SetShield(FMath::Clamp(GetShield(), 0.0f, GetMaxShield()));
	}
}

void UGSAttributeSetBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION_NOTIFY(UGSAttributeSetBase, Health, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UGSAttributeSetBase, MaxHealth, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UGSAttributeSetBase, HealthRegenRate, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UGSAttributeSetBase, Mana, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UGSAttributeSetBase, MaxMana, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UGSAttributeSetBase, ManaRegenRate, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UGSAttributeSetBase, Stamina, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UGSAttributeSetBase, MaxStamina, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UGSAttributeSetBase, StaminaRegenRate, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UGSAttributeSetBase, Shield, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UGSAttributeSetBase, MaxShield, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UGSAttributeSetBase, ShieldRegenRate, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UGSAttributeSetBase, Armor, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UGSAttributeSetBase, MoveSpeed, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UGSAttributeSetBase, CharacterLevel, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UGSAttributeSetBase, XP, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UGSAttributeSetBase, XPBounty, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UGSAttributeSetBase, Gold, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UGSAttributeSetBase, GoldBounty, COND_None, REPNOTIFY_Always);
}

void UGSAttributeSetBase::AdjustAttributeForMaxChange(FGameplayAttributeData& AffectedAttribute, const FGameplayAttributeData& MaxAttribute, float NewMaxValue, const FGameplayAttribute& AffectedAttributeProperty)
{
	UAbilitySystemComponent* AbilityComp = GetOwningAbilitySystemComponent();
	const float CurrentMaxValue = MaxAttribute.GetCurrentValue();
	if (!FMath::IsNearlyEqual(CurrentMaxValue, NewMaxValue) && AbilityComp)
	{
		// Change current value to maintain the current Val / Max percent
		const float CurrentValue = AffectedAttribute.GetCurrentValue();
		float NewDelta = (CurrentMaxValue > 0.f) ? (CurrentValue * NewMaxValue / CurrentMaxValue) - CurrentValue : NewMaxValue;

		AbilityComp->ApplyModToAttributeUnsafe(AffectedAttributeProperty, EGameplayModOp::Additive, NewDelta);
	}
}

void UGSAttributeSetBase::OnRep_Health()
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UGSAttributeSetBase, Health);
}

void UGSAttributeSetBase::OnRep_MaxHealth()
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UGSAttributeSetBase, MaxHealth);
}

void UGSAttributeSetBase::OnRep_HealthRegenRate()
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UGSAttributeSetBase, HealthRegenRate);
}

void UGSAttributeSetBase::OnRep_Mana()
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UGSAttributeSetBase, Mana);
}

void UGSAttributeSetBase::OnRep_MaxMana()
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UGSAttributeSetBase, MaxMana);
}

void UGSAttributeSetBase::OnRep_ManaRegenRate()
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UGSAttributeSetBase, ManaRegenRate);
}

void UGSAttributeSetBase::OnRep_Stamina()
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UGSAttributeSetBase, Stamina);
}

void UGSAttributeSetBase::OnRep_MaxStamina()
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UGSAttributeSetBase, MaxStamina);
}

void UGSAttributeSetBase::OnRep_StaminaRegenRate()
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UGSAttributeSetBase, StaminaRegenRate);
}

void UGSAttributeSetBase::OnRep_Shield()
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UGSAttributeSetBase, Shield);
}

void UGSAttributeSetBase::OnRep_MaxShield()
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UGSAttributeSetBase, MaxShield);
}

void UGSAttributeSetBase::OnRep_ShieldRegenRate()
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UGSAttributeSetBase, ShieldRegenRate);
}

void UGSAttributeSetBase::OnRep_Armor()
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UGSAttributeSetBase, Armor);
}

void UGSAttributeSetBase::OnRep_MoveSpeed()
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UGSAttributeSetBase, MoveSpeed);
}

void UGSAttributeSetBase::OnRep_CharacterLevel()
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UGSAttributeSetBase, CharacterLevel);
}

void UGSAttributeSetBase::OnRep_XP()
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UGSAttributeSetBase, XP);
}

void UGSAttributeSetBase::OnRep_XPBounty()
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UGSAttributeSetBase, XPBounty);
}

void UGSAttributeSetBase::OnRep_Gold()
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UGSAttributeSetBase, Gold);
}

void UGSAttributeSetBase::OnRep_GoldBounty()
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UGSAttributeSetBase, GoldBounty);
}
