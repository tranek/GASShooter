// Copyright 2019 Dan Kestranek.


#include "Weapons/GSWeapon.h"
#include "Characters/Abilities/GSAbilitySystemComponent.h"
#include "Characters/Abilities/GSGameplayAbility.h"
#include "Characters/Heroes/GSHeroCharacter.h"
#include "Components/SkeletalMeshComponent.h"
#include "Net/UnrealNetwork.h"

// Sets default values
AGSWeapon::AGSWeapon()
{
 	// Set this actor to never tick
	PrimaryActorTick.bCanEverTick = false;

	bReplicates = true;
	bNetUseOwnerRelevancy = true;

	Root = CreateDefaultSubobject<USceneComponent>(FName("Root"));
	Root->SetVisibility(false, false);
	RootComponent = Root;

	WeaponMesh1P = CreateDefaultSubobject<USkeletalMeshComponent>(FName("WeaponMesh1P"));
	//TODO for now no collision. No Collision while equipped, collision when sitting in the world waiting to be picked up.
	WeaponMesh1P->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	WeaponMesh1P->CastShadow = false;
	WeaponMesh1P->SetVisibility(false, true);
	WeaponMesh1P->SetupAttachment(RootComponent);

	WeaponMesh3P = CreateDefaultSubobject<USkeletalMeshComponent>(FName("WeaponMesh3P"));
	//TODO for now no collision. No Collision while equipped, collision when sitting in the world waiting to be picked up.
	WeaponMesh3P->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	WeaponMesh3P->SetupAttachment(RootComponent);
	WeaponMesh3P->bCastHiddenShadow = true;

	WeaponPrimaryInstantAbilityTag = FGameplayTag::RequestGameplayTag(FName("Ability.Weapon.Primary.Instant"));
	WeaponSecondaryInstantAbilityTag = FGameplayTag::RequestGameplayTag(FName("Ability.Weapon.Secondary.Instant"));
	WeaponAlternateInstantAbilityTag = FGameplayTag::RequestGameplayTag(FName("Ability.Weapon.Alternate.Instant"));
}

USkeletalMeshComponent* AGSWeapon::GetWeaponMesh1P()
{
	return WeaponMesh1P;
}

USkeletalMeshComponent* AGSWeapon::GetWeaponMesh3P()
{
	return WeaponMesh3P;
}

void AGSWeapon::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AGSWeapon, OwningCharacter);
}

void AGSWeapon::SetOwningCharacter(AGSHeroCharacter* InOwningCharacter)
{
	OwningCharacter = InOwningCharacter;
}

void AGSWeapon::NotifyActorBeginOverlap(AActor* Other)
{
	Super::NotifyActorBeginOverlap(Other);
	PickUpOnTouch(Cast<AGSHeroCharacter>(Other));
}

void AGSWeapon::Equip()
{
	if (!OwningCharacter)
	{
		UE_LOG(LogTemp, Error, TEXT("%s %s OwningCharacter is nullptr"), TEXT(__FUNCTION__), *GetName());
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("%s %s Role: %s"), TEXT(__FUNCTION__), *GetName(), GET_ACTOR_ROLE_FSTRING(OwningCharacter));

	FName AttachPoint = OwningCharacter->GetWeaponAttachPoint();

	Root->AttachToComponent(OwningCharacter->GetThirdPersonMesh(), FAttachmentTransformRules::KeepRelativeTransform, AttachPoint);

	if (WeaponMesh1P)
	{
		WeaponMesh1P->AttachToComponent(OwningCharacter->GetFirstPersonMesh(), FAttachmentTransformRules::SnapToTargetIncludingScale, AttachPoint);
		WeaponMesh1P->SetRelativeRotation(FRotator(0, 0, -90.0f));

		if (OwningCharacter->IsInFirstPersonPerspective())
		{
			WeaponMesh1P->SetVisibility(true, true);
		}
		else
		{
			WeaponMesh1P->SetVisibility(false, true);
		}
	}

	if (WeaponMesh3P)
	{
		WeaponMesh3P->AttachToComponent(OwningCharacter->GetThirdPersonMesh(), FAttachmentTransformRules::SnapToTargetIncludingScale, AttachPoint);
		WeaponMesh3P->SetRelativeRotation(FRotator(0, 0, -90.0f));

		if (OwningCharacter->IsInFirstPersonPerspective())
		{
			WeaponMesh3P->SetVisibility(false, true);
		}
		else
		{
			WeaponMesh3P->SetVisibility(true, true);
		}
	}

	AddAbilities();
}

void AGSWeapon::UnEquip()
{
	UE_LOG(LogTemp, Log, TEXT("%s %s %s"), TEXT(__FUNCTION__), *GetName(), GET_ACTOR_ROLE_FSTRING(OwningCharacter));

	WeaponMesh1P->DetachFromComponent(FDetachmentTransformRules::KeepRelativeTransform);
	WeaponMesh1P->SetVisibility(false, true);

	WeaponMesh3P->DetachFromComponent(FDetachmentTransformRules::KeepRelativeTransform);
	WeaponMesh3P->SetVisibility(false, true);

	RemoveAbilities();
}

void AGSWeapon::Reload()
{
	UE_LOG(LogTemp, Log, TEXT("%s %s"), TEXT(__FUNCTION__), *GetName());

}

int32 AGSWeapon::GetAbilityLevel(EGSAbilityInputID AbilityID)
{
	// All abilities for now are level 1
	return 1;
}

void AGSWeapon::PickUpOnTouch(AGSHeroCharacter* InCharacter)
{
	if (!InCharacter)
	{
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("%s %s %s"), TEXT(__FUNCTION__), *InCharacter->GetName(), *GetName());
}

void AGSWeapon::AddAbilities()
{
	UE_LOG(LogTemp, Log, TEXT("%s %s Role: %s"), TEXT(__FUNCTION__), *GetName(), GET_ACTOR_ROLE_FSTRING(OwningCharacter));

	if (!IsValid(OwningCharacter) || !OwningCharacter->GetAbilitySystemComponent())
	{
		return;
	}

	UGSAbilitySystemComponent* ASC = Cast<UGSAbilitySystemComponent>(OwningCharacter->GetAbilitySystemComponent());

	if (!ASC)
	{
		UE_LOG(LogTemp, Log, TEXT("%s %s Role: %s ASC is null"), TEXT(__FUNCTION__), *GetName(), GET_ACTOR_ROLE_FSTRING(OwningCharacter));
		return;
	}

	ASC->OnAbilityGiven.AddUObject(this, &AGSWeapon::HandleAbilityGiven);

	// Grant abilities, but only on the server	
	if (GetLocalRole() != ROLE_Authority)
	{
		return;
	}

	for (TSubclassOf<UGSGameplayAbility>& Ability : Abilities)
	{
		AbilitySpecHandles.Add(ASC->GiveAbility(
			FGameplayAbilitySpec(Ability, GetAbilityLevel(Ability.GetDefaultObject()->AbilityID), static_cast<int32>(Ability.GetDefaultObject()->AbilityInputID), this)));
	}
}

void AGSWeapon::RemoveAbilities()
{
	// These abilities will be cleared from the ASC when the abilities are removed on the Server
	PrimaryInstantAbilitySpecHandle = FGameplayAbilitySpecHandle();
	SecondaryInstantAbilitySpecHandle = FGameplayAbilitySpecHandle();
	AlternateInstantAbilitySpecHandle = FGameplayAbilitySpecHandle();
	PrimaryInstantAbilitySpec = nullptr;
	SecondaryInstantAbilitySpec = nullptr;
	AlternateInstantAbilitySpec = nullptr;
	PrimaryInstantAbility = nullptr;
	SecondaryInstantAbility = nullptr;
	AlternateInstantAbility = nullptr;

	if (!IsValid(OwningCharacter) || !OwningCharacter->GetAbilitySystemComponent())
	{
		return;
	}

	UGSAbilitySystemComponent* ASC = Cast<UGSAbilitySystemComponent>(OwningCharacter->GetAbilitySystemComponent());

	if (!ASC)
	{
		return;
	}

	ASC->OnAbilityGiven.RemoveAll(this);

	// Remove abilities, but only on the server	
	if (GetLocalRole() != ROLE_Authority)
	{
		return;
	}

	for (FGameplayAbilitySpecHandle& SpecHandle : AbilitySpecHandles)
	{
		ASC->ClearAbility(SpecHandle);
	}
}

void AGSWeapon::HandleAbilityGiven(FGameplayAbilitySpec& AbilitySpec)
{
	//UE_LOG(LogTemp, Log, TEXT("%s. %s. Ability: %s. Role: %s"), TEXT(__FUNCTION__), *GetName(), *AbilitySpec.GetDebugString(), GET_ACTOR_ROLE_FSTRING(OwningCharacter));

	if (IsValid(AbilitySpec.SourceObject) && AbilitySpec.SourceObject == this)
	{
		//  Cache the Weapon's cacheables
		if (AbilitySpec.Ability && AbilitySpec.Ability->AbilityTags.HasTagExact(WeaponPrimaryInstantAbilityTag))
		{
			PrimaryInstantAbilitySpecHandle = AbilitySpec.Handle;
			PrimaryInstantAbilitySpec = &AbilitySpec;
			PrimaryInstantAbility = Cast<UGSGameplayAbility>(AbilitySpec.GetPrimaryInstance());
		}
		else if (AbilitySpec.Ability && AbilitySpec.Ability->AbilityTags.HasTagExact(WeaponSecondaryInstantAbilityTag))
		{
			SecondaryInstantAbilitySpecHandle = AbilitySpec.Handle;
			SecondaryInstantAbilitySpec = &AbilitySpec;
			SecondaryInstantAbility = Cast<UGSGameplayAbility>(AbilitySpec.GetPrimaryInstance());
		}
		else if (AbilitySpec.Ability && AbilitySpec.Ability->AbilityTags.HasTagExact(WeaponAlternateInstantAbilityTag))
		{
			AlternateInstantAbilitySpecHandle = AbilitySpec.Handle;
			AlternateInstantAbilitySpec = &AbilitySpec;
			AlternateInstantAbility = Cast<UGSGameplayAbility>(AbilitySpec.GetPrimaryInstance());
		}
	}
}
