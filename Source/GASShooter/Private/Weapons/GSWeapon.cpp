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
	}

	if (WeaponMesh3P)
	{
		WeaponMesh3P->AttachToComponent(OwningCharacter->GetThirdPersonMesh(), FAttachmentTransformRules::SnapToTargetIncludingScale, AttachPoint);
		WeaponMesh3P->SetRelativeRotation(FRotator(0, 0, -90.0f));

		if (OwningCharacter->IsInFirstPersonPerspective())
		{
			WeaponMesh3P->SetVisibility(false, true);
		}
	}

	AddAbilities();
}

void AGSWeapon::UnEquip()
{
	UE_LOG(LogTemp, Log, TEXT("%s %s"), TEXT(__FUNCTION__), *GetName());

	WeaponMesh1P->DetachFromComponent(FDetachmentTransformRules::KeepRelativeTransform);
	WeaponMesh1P->SetVisibility(false, false);

	WeaponMesh3P->DetachFromComponent(FDetachmentTransformRules::KeepRelativeTransform);
	WeaponMesh3P->SetVisibility(false, false);

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
	// Grant abilities, but only on the server	
	if (GetLocalRole() != ROLE_Authority || !IsValid(OwningCharacter) || !OwningCharacter->GetAbilitySystemComponent())
	{
		return;
	}

	UGSAbilitySystemComponent* ASC = Cast<UGSAbilitySystemComponent>(OwningCharacter->GetAbilitySystemComponent());

	for (TSubclassOf<UGSGameplayAbility>& Ability : Abilities)
	{
		AbilitySpecs.Add(ASC->GiveAbility(
			FGameplayAbilitySpec(Ability, GetAbilityLevel(Ability.GetDefaultObject()->AbilityID), static_cast<int32>(Ability.GetDefaultObject()->AbilityInputID), this)));
	}
}

void AGSWeapon::RemoveAbilities()
{
	// Remove abilities, but only on the server	
	if (GetLocalRole() != ROLE_Authority || !IsValid(OwningCharacter) || !OwningCharacter->GetAbilitySystemComponent())
	{
		return;
	}

	UGSAbilitySystemComponent* ASC = Cast<UGSAbilitySystemComponent>(OwningCharacter->GetAbilitySystemComponent());

	for (FGameplayAbilitySpecHandle SpecHandle : AbilitySpecs)
	{
		ASC->ClearAbility(SpecHandle);
	}
}
