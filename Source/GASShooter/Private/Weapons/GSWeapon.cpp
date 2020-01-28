// Copyright 2020 Dan Kestranek.


#include "Weapons/GSWeapon.h"
#include "Characters/Abilities/GSAbilitySystemComponent.h"
#include "Characters/Abilities/GSGameplayAbility.h"
#include "Characters/Abilities/GSGATA_SingleLineTrace.h"
#include "Characters/Heroes/GSHeroCharacter.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "GSBlueprintFunctionLibrary.h"
#include "Net/UnrealNetwork.h"
#include "Player/GSPlayerController.h"

// Sets default values
AGSWeapon::AGSWeapon()
{
 	// Set this actor to never tick
	PrimaryActorTick.bCanEverTick = false;

	bReplicates = true;
	bNetUseOwnerRelevancy = true;
	NetUpdateFrequency = 100.0f; // Set this to a value that's appropriate for your game
	bSpawnWithCollision = true;
	PrimaryClipAmmo = 0;
	MaxPrimaryClipAmmo = 0;
	SecondaryClipAmmo = 0;
	MaxSecondaryClipAmmo = 0;
	bInfiniteAmmo = false;
	PrimaryAmmoType = FGameplayTag::RequestGameplayTag(FName("Weapon.Ammo.None"));
	SecondaryAmmoType = FGameplayTag::RequestGameplayTag(FName("Weapon.Ammo.None"));
	bEnableSingleLineTraceTargetActor = false;
	bEnableSphereTraceTargetActor = false;

	CollisionComp = CreateDefaultSubobject<UCapsuleComponent>(FName("CollisionComponent"));
	CollisionComp->InitCapsuleSize(40.0f, 50.0f);
	CollisionComp->SetCollisionObjectType(COLLISION_PICKUP);
	CollisionComp->SetCollisionEnabled(ECollisionEnabled::NoCollision); // Manually enable when in pickup mode
	CollisionComp->SetCollisionResponseToAllChannels(ECR_Ignore);
	CollisionComp->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	RootComponent = CollisionComp;

	WeaponMesh1P = CreateDefaultSubobject<USkeletalMeshComponent>(FName("WeaponMesh1P"));
	WeaponMesh1P->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	WeaponMesh1P->CastShadow = false;
	WeaponMesh1P->SetVisibility(false, true);
	WeaponMesh1P->SetupAttachment(CollisionComp);
	WeaponMesh1P->VisibilityBasedAnimTickOption = EVisibilityBasedAnimTickOption::AlwaysTickPose;

	WeaponMesh3P = CreateDefaultSubobject<USkeletalMeshComponent>(FName("WeaponMesh3P"));
	WeaponMesh3P->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	WeaponMesh3P->SetupAttachment(CollisionComp);
	WeaponMesh3P->CastShadow = true;
	WeaponMesh3P->SetVisibility(true, true);
	WeaponMesh3P->VisibilityBasedAnimTickOption = EVisibilityBasedAnimTickOption::AlwaysTickPose;

	WeaponPrimaryInstantAbilityTag = FGameplayTag::RequestGameplayTag(FName("Ability.Weapon.Primary.Instant"));
	WeaponSecondaryInstantAbilityTag = FGameplayTag::RequestGameplayTag(FName("Ability.Weapon.Secondary.Instant"));
	WeaponAlternateInstantAbilityTag = FGameplayTag::RequestGameplayTag(FName("Ability.Weapon.Alternate.Instant"));
	WeaponIsFiringTag = FGameplayTag::RequestGameplayTag(FName("Weapon.IsFiring"));

	FireMode = FGameplayTag::RequestGameplayTag(FName("Weapon.FireMode.None"));
}

UAbilitySystemComponent* AGSWeapon::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

USkeletalMeshComponent* AGSWeapon::GetWeaponMesh1P() const
{
	return WeaponMesh1P;
}

USkeletalMeshComponent* AGSWeapon::GetWeaponMesh3P() const
{
	return WeaponMesh3P;
}

void AGSWeapon::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AGSWeapon, OwningCharacter);
	DOREPLIFETIME(AGSWeapon, PrimaryClipAmmo);
	DOREPLIFETIME(AGSWeapon, MaxPrimaryClipAmmo);
	DOREPLIFETIME(AGSWeapon, SecondaryClipAmmo);
	DOREPLIFETIME(AGSWeapon, MaxSecondaryClipAmmo);
}

void AGSWeapon::PreReplication(IRepChangedPropertyTracker& ChangedPropertyTracker)
{
	Super::PreReplication(ChangedPropertyTracker);

	DOREPLIFETIME_ACTIVE_OVERRIDE(AGSWeapon, PrimaryClipAmmo, (IsValid(AbilitySystemComponent) && !AbilitySystemComponent->HasMatchingGameplayTag(WeaponIsFiringTag)) || bPrimaryAmmoNeedsSync);
	DOREPLIFETIME_ACTIVE_OVERRIDE(AGSWeapon, SecondaryClipAmmo, (IsValid(AbilitySystemComponent) && !AbilitySystemComponent->HasMatchingGameplayTag(WeaponIsFiringTag)) || bPrimaryAmmoNeedsSync);
	
	if (bPrimaryAmmoNeedsSync)
	{
		bPrimaryAmmoNeedsSync = false;
	}

	if (bSecondaryAmmoNeedsSync)
	{
		bSecondaryAmmoNeedsSync = false;
	}
}

void AGSWeapon::SetOwningCharacter(AGSHeroCharacter* InOwningCharacter)
{
	OwningCharacter = InOwningCharacter;
	if (OwningCharacter)
	{
		// Called when added to inventory
		AbilitySystemComponent = Cast<UGSAbilitySystemComponent>(OwningCharacter->GetAbilitySystemComponent());
		SetOwner(InOwningCharacter);
		CollisionComp->AttachToComponent(OwningCharacter->GetRootComponent(), FAttachmentTransformRules::KeepRelativeTransform);
		CollisionComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);

		if (OwningCharacter->GetCurrentWeapon() != this)
		{
			WeaponMesh3P->CastShadow = false;
			WeaponMesh3P->SetVisibility(true, true);
			WeaponMesh3P->SetVisibility(false, true);
		}
	}
	else
	{
		AbilitySystemComponent = nullptr;
		SetOwner(nullptr);
		CollisionComp->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);
	}
}

void AGSWeapon::NotifyActorBeginOverlap(AActor* Other)
{
	Super::NotifyActorBeginOverlap(Other);

	if (!IsPendingKill() && !OwningCharacter)
	{
		PickUpOnTouch(Cast<AGSHeroCharacter>(Other));
	}
}

void AGSWeapon::Equip()
{
	if (!OwningCharacter)
	{
		UE_LOG(LogTemp, Error, TEXT("%s %s OwningCharacter is nullptr"), TEXT(__FUNCTION__), *GetName());
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("%s %s %s Role: %s"), TEXT(__FUNCTION__), *UGSBlueprintFunctionLibrary::GetPlayerEditorWindowRole(GetWorld()), *GetName(), GET_ACTOR_ROLE_FSTRING(OwningCharacter));

	FName AttachPoint = OwningCharacter->GetWeaponAttachPoint();

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
		WeaponMesh3P->CastShadow = true;
		WeaponMesh3P->bCastHiddenShadow = true;

		if (OwningCharacter->IsInFirstPersonPerspective())
		{
			WeaponMesh3P->SetVisibility(true, true); // Without this, the weapon's 3p shadow doesn't show
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
	UE_LOG(LogTemp, Log, TEXT("%s %s %s"), TEXT(__FUNCTION__), *GetName(), *UGSBlueprintFunctionLibrary::GetPlayerEditorWindowRole(GetWorld()));

	if (OwningCharacter == nullptr)
	{
		//UE_LOG(LogTemp, Warning, TEXT("%s %s %s OwningCharacter was nullptr"), TEXT(__FUNCTION__), *GetName(),
		//	*UGSBlueprintFunctionLibrary::GetPlayerEditorWindowRole(GetWorld()));
		return;
	}

	//WeaponMesh1P->DetachFromComponent(FDetachmentTransformRules::KeepRelativeTransform);
	WeaponMesh1P->SetVisibility(false, true);

	//WeaponMesh3P->DetachFromComponent(FDetachmentTransformRules::KeepRelativeTransform);
	WeaponMesh3P->CastShadow = false;
	WeaponMesh3P->bCastHiddenShadow = false;
	WeaponMesh3P->SetVisibility(true, true); // Without this, the unequipped weapon's 3p shadow hangs around
	WeaponMesh3P->SetVisibility(false, true);
}

void AGSWeapon::AddAbilities()
{
	UE_LOG(LogTemp, Log, TEXT("%s %s %s Role: %s"), TEXT(__FUNCTION__), *UGSBlueprintFunctionLibrary::GetPlayerEditorWindowRole(GetWorld()), *GetName(), GET_ACTOR_ROLE_FSTRING(OwningCharacter));

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

void AGSWeapon::OnDropped_Implementation(FVector NewLocation)
{
	UE_LOG(LogTemp, Log, TEXT("%s %s %s"), TEXT(__FUNCTION__), *UGSBlueprintFunctionLibrary::GetPlayerEditorWindowRole(GetWorld()), *GetName());

	SetOwningCharacter(nullptr);
	ResetWeapon();

	SetActorLocation(NewLocation);
	//CollisionComp->SetHiddenInGame(false, false);
	CollisionComp->SetCollisionEnabled(ECollisionEnabled::QueryOnly);

	if (WeaponMesh1P)
	{
		WeaponMesh1P->AttachToComponent(CollisionComp, FAttachmentTransformRules::SnapToTargetIncludingScale);
		WeaponMesh1P->SetVisibility(false, true);
	}

	if (WeaponMesh3P)
	{
		WeaponMesh3P->AttachToComponent(CollisionComp, FAttachmentTransformRules::SnapToTargetIncludingScale);
		WeaponMesh3P->SetRelativeLocation(FVector(0.0f, -25.0f, 0.0f));
		WeaponMesh3P->CastShadow = true;
		WeaponMesh3P->SetVisibility(true, true);
	}
}

bool AGSWeapon::OnDropped_Validate(FVector NewLocation)
{
	return true;
}

int32 AGSWeapon::GetPrimaryClipAmmo() const
{
	return PrimaryClipAmmo;
}

int32 AGSWeapon::GetMaxPrimaryClipAmmo() const
{
	return MaxPrimaryClipAmmo;
}

int32 AGSWeapon::GetSecondaryClipAmmo() const
{
	return SecondaryClipAmmo;
}

int32 AGSWeapon::GetMaxSecondaryClipAmmo() const
{
	return MaxSecondaryClipAmmo;
}

void AGSWeapon::SetPrimaryClipAmmo(int32 NewPrimaryClipAmmo)
{
	int32 OldPrimaryClipAmmo = PrimaryClipAmmo;
	PrimaryClipAmmo = NewPrimaryClipAmmo;
	OnPrimaryClipAmmoChanged.Broadcast(OldPrimaryClipAmmo, PrimaryClipAmmo);
}

void AGSWeapon::SetMaxPrimaryClipAmmo(int32 NewMaxPrimaryClipAmmo)
{
	int32 OldMaxPrimaryClipAmmo = MaxPrimaryClipAmmo;
	MaxPrimaryClipAmmo = NewMaxPrimaryClipAmmo;
	OnMaxPrimaryClipAmmoChanged.Broadcast(OldMaxPrimaryClipAmmo, MaxPrimaryClipAmmo);
}

void AGSWeapon::SetSecondaryClipAmmo(int32 NewSecondaryClipAmmo)
{
	int32 OldSecondaryClipAmmo = SecondaryClipAmmo;
	SecondaryClipAmmo = NewSecondaryClipAmmo;
	OnSecondaryClipAmmoChanged.Broadcast(OldSecondaryClipAmmo, SecondaryClipAmmo);
}

void AGSWeapon::SetMaxSecondaryClipAmmo(int32 NewMaxSecondaryClipAmmo)
{
	int32 OldMaxSecondaryClipAmmo = MaxSecondaryClipAmmo;
	MaxSecondaryClipAmmo = NewMaxSecondaryClipAmmo;
	OnMaxSecondaryClipAmmoChanged.Broadcast(OldMaxSecondaryClipAmmo, MaxSecondaryClipAmmo);
}

TSubclassOf<UGSHUDReticle> AGSWeapon::GetPrimaryHUDReticleClass() const
{
	return PrimaryHUDReticleClass;
}

bool AGSWeapon::HasInfiniteAmmo() const
{
	return bInfiniteAmmo;
}

UAnimMontage* AGSWeapon::GetEquip1PMontage() const
{
	return Equip1PMontage;
}

UAnimMontage* AGSWeapon::GetEquip3PMontage() const
{
	return Equip3PMontage;
}

void AGSWeapon::BeginPlay()
{
	UE_LOG(LogTemp, Log, TEXT("%s %s %s"), TEXT(__FUNCTION__), *GetName(), *UGSBlueprintFunctionLibrary::GetPlayerEditorWindowRole(GetWorld()));

	ResetWeapon();

	if (bEnableSingleLineTraceTargetActor)
	{
		SingleLineTraceTargetActor = GetWorld()->SpawnActor<AGSGATA_SingleLineTrace>();
		SingleLineTraceTargetActor->SetOwner(this);
	}

	if (bEnableSphereTraceTargetActor)
	{
		//TODO
	}

	if (!OwningCharacter && bSpawnWithCollision)
	{
		// Spawned into the world without an owner, enable collision as we are in pickup mode
		UE_LOG(LogTemp, Log, TEXT("%s %s %s Spawning with collision enabled"), TEXT(__FUNCTION__), *GetName(),
			*UGSBlueprintFunctionLibrary::GetPlayerEditorWindowRole(GetWorld()));
		CollisionComp->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	}

	Super::BeginPlay();
}

void AGSWeapon::PickUpOnTouch(AGSHeroCharacter* InCharacter)
{
	if (!InCharacter || !InCharacter->IsAlive())
	{
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("%s %s %s %s"), TEXT(__FUNCTION__), *InCharacter->GetName(), *GetName(),
		*UGSBlueprintFunctionLibrary::GetPlayerEditorWindowRole(GetWorld()));

	if (InCharacter->AddWeaponToInventory(this, true))
	{
		WeaponMesh3P->CastShadow = false;
		WeaponMesh3P->SetVisibility(true, true);
		WeaponMesh3P->SetVisibility(false, true);
	}
}

void AGSWeapon::OnRep_PrimaryClipAmmo(int32 OldPrimaryClipAmmo)
{
	OnPrimaryClipAmmoChanged.Broadcast(OldPrimaryClipAmmo, PrimaryClipAmmo);
}

void AGSWeapon::OnRep_MaxPrimaryClipAmmo(int32 OldMaxPrimaryClipAmmo)
{
	OnMaxPrimaryClipAmmoChanged.Broadcast(OldMaxPrimaryClipAmmo, MaxPrimaryClipAmmo);
}

void AGSWeapon::OnRep_SecondaryClipAmmo(int32 OldSecondaryClipAmmo)
{
	OnSecondaryClipAmmoChanged.Broadcast(OldSecondaryClipAmmo, SecondaryClipAmmo);
}

void AGSWeapon::OnRep_MaxSecondaryClipAmmo(int32 OldMaxSecondaryClipAmmo)
{
	OnMaxSecondaryClipAmmoChanged.Broadcast(OldMaxSecondaryClipAmmo, MaxSecondaryClipAmmo);
}
