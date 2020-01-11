// Copyright 2020 Dan Kestranek.


#include "Weapons/GSWeapon.h"
#include "Characters/Abilities/GSAbilitySystemComponent.h"
#include "Characters/Abilities/GSGameplayAbility.h"
#include "Characters/Abilities/GSGATA_SingleLineTrace.h"
#include "Characters/Heroes/GSHeroCharacter.h"
#include "Components/SkeletalMeshComponent.h"
#include "Net/UnrealNetwork.h"
#include "Weapons/GSWeaponAttributeSet.h"

// Sets default values
AGSWeapon::AGSWeapon()
{
 	// Set this actor to never tick
	PrimaryActorTick.bCanEverTick = false;

	bReplicates = true;
	bNetUseOwnerRelevancy = true;
	// Set this to a value that's appropriate for your game
	NetUpdateFrequency = 100.0f;

	Root = CreateDefaultSubobject<USceneComponent>(FName("Root"));
	Root->SetVisibility(false, false);
	RootComponent = Root;

	WeaponMesh1P = CreateDefaultSubobject<USkeletalMeshComponent>(FName("WeaponMesh1P"));
	//TODO for now no collision. No Collision while equipped, collision when sitting in the world waiting to be picked up.
	WeaponMesh1P->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	WeaponMesh1P->CastShadow = false;
	WeaponMesh1P->SetVisibility(false, true);
	WeaponMesh1P->SetupAttachment(RootComponent);
	WeaponMesh1P->VisibilityBasedAnimTickOption = EVisibilityBasedAnimTickOption::AlwaysTickPose;

	WeaponMesh3P = CreateDefaultSubobject<USkeletalMeshComponent>(FName("WeaponMesh3P"));
	//TODO for now no collision. No Collision while equipped, collision when sitting in the world waiting to be picked up.
	WeaponMesh3P->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	WeaponMesh3P->SetupAttachment(RootComponent);
	WeaponMesh3P->bCastHiddenShadow = true;
	WeaponMesh3P->VisibilityBasedAnimTickOption = EVisibilityBasedAnimTickOption::AlwaysTickPose;

	WeaponPrimaryInstantAbilityTag = FGameplayTag::RequestGameplayTag(FName("Ability.Weapon.Primary.Instant"));
	WeaponSecondaryInstantAbilityTag = FGameplayTag::RequestGameplayTag(FName("Ability.Weapon.Secondary.Instant"));
	WeaponAlternateInstantAbilityTag = FGameplayTag::RequestGameplayTag(FName("Ability.Weapon.Alternate.Instant"));

	FireMode = FGameplayTag::RequestGameplayTag(FName("Weapon.FireMode.None"));
}

UAbilitySystemComponent* AGSWeapon::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
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
	DOREPLIFETIME(AGSWeapon, AttributeSet);
}

void AGSWeapon::SetOwningCharacter(AGSHeroCharacter* InOwningCharacter)
{
	OwningCharacter = InOwningCharacter;
	if (OwningCharacter)
	{
		AbilitySystemComponent = Cast<UGSAbilitySystemComponent>(OwningCharacter->GetAbilitySystemComponent());
	}
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
}

void AGSWeapon::UnEquip()
{
	UE_LOG(LogTemp, Log, TEXT("%s %s"), TEXT(__FUNCTION__), *GetName());

	WeaponMesh1P->DetachFromComponent(FDetachmentTransformRules::KeepRelativeTransform);
	WeaponMesh1P->SetVisibility(false, true);

	WeaponMesh3P->DetachFromComponent(FDetachmentTransformRules::KeepRelativeTransform);
	WeaponMesh3P->SetVisibility(false, true);
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
	if (!IsValid(OwningCharacter) || !OwningCharacter->GetAbilitySystemComponent())
	{
		return;
	}

	UGSAbilitySystemComponent* ASC = Cast<UGSAbilitySystemComponent>(OwningCharacter->GetAbilitySystemComponent());

	if (!ASC)
	{
		return;
	}

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

void AGSWeapon::Reload()
{
	UE_LOG(LogTemp, Log, TEXT("%s %s"), TEXT(__FUNCTION__), *GetName());

}

int32 AGSWeapon::GetAbilityLevel(EGSAbilityInputID AbilityID)
{
	// All abilities for now are level 1
	return 1;
}

void AGSWeapon::ResetWeapon()
{
	FireMode = DefaultFireMode;
	StatusText = DefaultStatusText;
}

int32 AGSWeapon::GetPrimaryClipAmmo() const
{
	if (IsValid(AttributeSet))
	{
		return AttributeSet->GetPrimaryClipAmmo();
	}

	return 0;
}

int32 AGSWeapon::GetPrimaryMaxClipAmmo() const
{
	if (IsValid(AttributeSet))
	{
		return AttributeSet->GetPrimaryMaxClipAmmo();
	}

	return 0;
}

int32 AGSWeapon::GetPrimaryReserveAmmo() const
{
	if (IsValid(AttributeSet))
	{
		return AttributeSet->GetPrimaryReserveAmmo();
	}

	return 0;
}

int32 AGSWeapon::GetSecondaryClipAmmo() const
{
	if (IsValid(AttributeSet))
	{
		return AttributeSet->GetSecondaryClipAmmo();
	}

	return 0;
}

int32 AGSWeapon::GetSecondaryMaxClipAmmo() const
{
	if (IsValid(AttributeSet))
	{
		return AttributeSet->GetSecondaryMaxClipAmmo();
	}

	return 0;
}

int32 AGSWeapon::GetSecondaryReserveAmmo() const
{
	if (IsValid(AttributeSet))
	{
		return AttributeSet->GetSecondaryReserveAmmo();
	}

	return 0;
}

void AGSWeapon::BeginPlay()
{
	AttributeSet = NewObject<UGSWeaponAttributeSet>(this);

	ResetWeapon();

	SingleLineTraceTargetActor = GetWorld()->SpawnActor<AGSGATA_SingleLineTrace>();

	// Init Attributes on Server, they will be replicated to clients
	if (GetLocalRole() == ROLE_Authority && AttributeSet)
	{
		AttributeSet->InitPrimaryMaxAmmo(PrimaryMaxAmmo);
		AttributeSet->InitPrimaryMaxClipAmmo(PrimaryMaxClipAmmo);
		AttributeSet->InitPrimaryClipAmmo(PrimaryClipAmmo);
		AttributeSet->InitPrimaryReserveAmmo(PrimaryReserveAmmo);
		AttributeSet->InitPrimaryDamage(PrimaryDamage);
		AttributeSet->InitSecondaryMaxAmmo(SecondaryMaxAmmo);
		AttributeSet->InitSecondaryMaxClipAmmo(SecondaryMaxClipAmmo);
		AttributeSet->InitSecondaryClipAmmo(SecondaryClipAmmo);
		AttributeSet->InitSecondaryReserveAmmo(SecondaryReserveAmmo);
		AttributeSet->InitSecondaryDamage(SecondaryDamage);
	}
}

void AGSWeapon::PickUpOnTouch(AGSHeroCharacter* InCharacter)
{
	if (!InCharacter)
	{
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("%s %s %s"), TEXT(__FUNCTION__), *InCharacter->GetName(), *GetName());

	//TODO call add to inventory on incharacter. Check if alive first.
}
