// Copyright 2020 Dan Kestranek.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "AbilitySystemInterface.h"
#include "GameplayAbilitySpec.h"
#include "GameplayTagContainer.h"
#include "GASShooter/GASShooter.h"
#include "GSWeapon.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FWeaponAmmoChangedDelegate, int32, OldValue, int32, NewValue);

class AGSGATA_LineTrace;
class AGSGATA_SphereTrace;
class AGSHeroCharacter;
class UAnimMontage;
class UGSAbilitySystemComponent;
class UGSGameplayAbility;
class UPaperSprite;
class USkeletalMeshComponent;

UCLASS(Blueprintable, BlueprintType)
class GASSHOOTER_API AGSWeapon : public AActor, public IAbilitySystemInterface
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AGSWeapon();

	// Whether or not to spawn this weapon with collision enabled (pickup mode).
	// Set to false when spawning directly into a player's inventory or true when spawning into the world in pickup mode.
	UPROPERTY(BlueprintReadWrite)
	bool bSpawnWithCollision;

	// This tag is primarily used by the first person Animation Blueprint to determine which animations to play
	// (Rifle vs Rocket Launcher)
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "GASShooter|GSWeapon")
	FGameplayTag WeaponTag;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "GASShooter|GSWeapon")
	FGameplayTagContainer RestrictedPickupTags;
	
	// UI HUD Primary Icon when equipped. Using Sprites because of the texture atlas from ShooterGame.
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "GASShooter|UI")
	UPaperSprite* PrimaryIcon;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "GASShooter|UI")
	UPaperSprite* SecondaryIcon;

	// UI HUD Primary Clip Icon when equipped
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "GASShooter|UI")
	UPaperSprite* PrimaryClipIcon;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "GASShooter|UI")
	UPaperSprite* SecondaryClipIcon;

	UPROPERTY(BlueprintReadWrite, VisibleInstanceOnly, Category = "GASShooter|GSWeapon")
	FGameplayTag FireMode;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "GASShooter|GSWeapon")
	FGameplayTag PrimaryAmmoType;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "GASShooter|GSWeapon")
	FGameplayTag SecondaryAmmoType;

	// Things like fire mode for rifle
	UPROPERTY(BlueprintReadWrite, VisibleInstanceOnly, Category = "GASShooter|GSWeapon")
	FText StatusText;

	UPROPERTY(BlueprintAssignable, Category = "GASShooter|GSWeapon")
	FWeaponAmmoChangedDelegate OnPrimaryClipAmmoChanged;

	UPROPERTY(BlueprintAssignable, Category = "GASShooter|GSWeapon")
	FWeaponAmmoChangedDelegate OnMaxPrimaryClipAmmoChanged;

	UPROPERTY(BlueprintAssignable, Category = "GASShooter|GSWeapon")
	FWeaponAmmoChangedDelegate OnSecondaryClipAmmoChanged;

	UPROPERTY(BlueprintAssignable, Category = "GASShooter|GSWeapon")
	FWeaponAmmoChangedDelegate OnMaxSecondaryClipAmmoChanged;

	// Implement IAbilitySystemInterface
	virtual class UAbilitySystemComponent* GetAbilitySystemComponent() const override;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "GASShooter|GSWeapon")
	virtual USkeletalMeshComponent* GetWeaponMesh1P() const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "GASShooter|GSWeapon")
	virtual USkeletalMeshComponent* GetWeaponMesh3P() const;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	virtual void PreReplication(IRepChangedPropertyTracker& ChangedPropertyTracker) override;

	void SetOwningCharacter(AGSHeroCharacter* InOwningCharacter);

	// Pickup on touch
	virtual void NotifyActorBeginOverlap(class AActor* Other) override;

	// Called when the player equips this weapon
	virtual void Equip();

	// Called when the player unequips this weapon
	virtual void UnEquip();

	virtual void AddAbilities();

	virtual void RemoveAbilities();

	virtual int32 GetAbilityLevel(EGSAbilityInputID AbilityID);

	// Resets things like fire mode to default
	UFUNCTION(BlueprintCallable, Category = "GASShooter|GSWeapon")
	virtual void ResetWeapon();

	UFUNCTION(NetMulticast, Reliable)
	void OnDropped(FVector NewLocation);
	virtual void OnDropped_Implementation(FVector NewLocation);
	virtual bool OnDropped_Validate(FVector NewLocation);

	UFUNCTION(BlueprintCallable, Category = "GASShooter|GSWeapon")
	virtual int32 GetPrimaryClipAmmo() const;

	UFUNCTION(BlueprintCallable, Category = "GASShooter|GSWeapon")
	virtual int32 GetMaxPrimaryClipAmmo() const;

	UFUNCTION(BlueprintCallable, Category = "GASShooter|GSWeapon")
	virtual int32 GetSecondaryClipAmmo() const;

	UFUNCTION(BlueprintCallable, Category = "GASShooter|GSWeapon")
	virtual int32 GetMaxSecondaryClipAmmo() const;

	UFUNCTION(BlueprintCallable, Category = "GASShooter|GSWeapon")
	virtual void SetPrimaryClipAmmo(int32 NewPrimaryClipAmmo);

	UFUNCTION(BlueprintCallable, Category = "GASShooter|GSWeapon")
	virtual void SetMaxPrimaryClipAmmo(int32 NewMaxPrimaryClipAmmo);

	UFUNCTION(BlueprintCallable, Category = "GASShooter|GSWeapon")
	virtual void SetSecondaryClipAmmo(int32 NewSecondaryClipAmmo);

	UFUNCTION(BlueprintCallable, Category = "GASShooter|GSWeapon")
	virtual void SetMaxSecondaryClipAmmo(int32 NewMaxSecondaryClipAmmo);

	UFUNCTION(BlueprintCallable, Category = "GASShooter|GSWeapon")
	TSubclassOf<class UGSHUDReticle> GetPrimaryHUDReticleClass() const;

	UFUNCTION(BlueprintCallable, Category = "GASShooter|GSWeapon")
	virtual bool HasInfiniteAmmo() const;

	UFUNCTION(BlueprintCallable, Category = "GASShooter|Animation")
	UAnimMontage* GetEquip1PMontage() const;

	UFUNCTION(BlueprintCallable, Category = "GASShooter|Animation")
	UAnimMontage* GetEquip3PMontage() const;
	
	UFUNCTION(BlueprintCallable, Category = "GASShooter|Audio")
	class USoundCue* GetPickupSound() const;

	UFUNCTION(BlueprintCallable, Category = "GASShooter|GSWeapon")
	FText GetDefaultStatusText() const;

	// Getter for LineTraceTargetActor. Spawns it if it doesn't exist yet.
	UFUNCTION(BlueprintCallable, Category = "GASShooter|Targeting")
	AGSGATA_LineTrace* GetLineTraceTargetActor();

	// Getter for SphereTraceTargetActor. Spawns it if it doesn't exist yet.
	UFUNCTION(BlueprintCallable, Category = "GASShooter|Targeting")
	AGSGATA_SphereTrace* GetSphereTraceTargetActor();

protected:
	UPROPERTY()
	UGSAbilitySystemComponent* AbilitySystemComponent;

	// How much ammo in the clip the gun starts with
	UPROPERTY(BlueprintReadOnly, EditAnywhere, ReplicatedUsing = OnRep_PrimaryClipAmmo, Category = "GASShooter|GSWeapon|Ammo")
	int32 PrimaryClipAmmo;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, ReplicatedUsing = OnRep_MaxPrimaryClipAmmo, Category = "GASShooter|GSWeapon|Ammo")
	int32 MaxPrimaryClipAmmo;

	// How much ammo in the clip the gun starts with. Used for things like rifle grenades.
	UPROPERTY(BlueprintReadOnly, EditAnywhere, ReplicatedUsing = OnRep_SecondaryClipAmmo, Category = "GASShooter|GSWeapon|Ammo")
	int32 SecondaryClipAmmo;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, ReplicatedUsing = OnRep_MaxSecondaryClipAmmo, Category = "GASShooter|GSWeapon|Ammo")
	int32 MaxSecondaryClipAmmo;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "GASShooter|GSWeapon|Ammo")
	bool bInfiniteAmmo;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "GASShooter|UI")
	TSubclassOf<class UGSHUDReticle> PrimaryHUDReticleClass;

	UPROPERTY()
	AGSGATA_LineTrace* LineTraceTargetActor;

	UPROPERTY()
	AGSGATA_SphereTrace* SphereTraceTargetActor;

	// Collision capsule for when weapon is in pickup mode
	UPROPERTY(VisibleAnywhere)
	class UCapsuleComponent* CollisionComp;

	UPROPERTY(VisibleAnywhere, Category = "GASShooter|GSWeapon")
	USkeletalMeshComponent* WeaponMesh1P;

	UPROPERTY(VisibleAnywhere, Category = "GASShooter|GSWeapon")
	USkeletalMeshComponent* WeaponMesh3P;

	// Relative Location of weapon 3P Mesh when in pickup mode
	// 1P weapon mesh is invisible so it doesn't need one
	UPROPERTY(EditDefaultsOnly, Category = "GASShooter|GSWeapon")
	FVector WeaponMesh3PickupRelativeLocation;

	// Relative Location of weapon 1P Mesh when equipped
	UPROPERTY(EditDefaultsOnly, Category = "GASShooter|GSWeapon")
	FVector WeaponMesh1PEquippedRelativeLocation;

	// Relative Location of weapon 3P Mesh when equipped
	UPROPERTY(EditDefaultsOnly, Category = "GASShooter|GSWeapon")
	FVector WeaponMesh3PEquippedRelativeLocation;

	UPROPERTY(BlueprintReadOnly, Replicated, Category = "GASShooter|GSWeapon")
	AGSHeroCharacter* OwningCharacter;

	UPROPERTY(EditAnywhere, Category = "GASShooter|GSWeapon")
	TArray<TSubclassOf<UGSGameplayAbility>> Abilities;

	UPROPERTY(BlueprintReadOnly, Category = "GASShooter|GSWeapon")
	TArray<FGameplayAbilitySpecHandle> AbilitySpecHandles;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "GASShooter|GSWeapon")
	FGameplayTag DefaultFireMode;

	// Things like fire mode for rifle
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "GASShooter|GSWeapon")
	FText DefaultStatusText;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "GASShooter|Animation")
	UAnimMontage* Equip1PMontage;

	UPROPERTY(BlueprintReadonly, EditAnywhere, Category = "GASShooter|Animation")
	UAnimMontage* Equip3PMontage;

	// Sound played when player picks it up
	UPROPERTY(EditDefaultsOnly, Category = "GASShooter|Audio")
	class USoundCue* PickupSound;

	// Cache tags
	FGameplayTag WeaponPrimaryInstantAbilityTag;
	FGameplayTag WeaponSecondaryInstantAbilityTag;
	FGameplayTag WeaponAlternateInstantAbilityTag;
	FGameplayTag WeaponIsFiringTag;

	virtual void BeginPlay() override;
	virtual void EndPlay(EEndPlayReason::Type EndPlayReason) override;

	// Called when the player picks up this weapon
	virtual void PickUpOnTouch(AGSHeroCharacter* InCharacter);

	UFUNCTION()
	virtual void OnRep_PrimaryClipAmmo(int32 OldPrimaryClipAmmo);

	UFUNCTION()
	virtual void OnRep_MaxPrimaryClipAmmo(int32 OldMaxPrimaryClipAmmo);

	UFUNCTION()
	virtual void OnRep_SecondaryClipAmmo(int32 OldSecondaryClipAmmo);

	UFUNCTION()
	virtual void OnRep_MaxSecondaryClipAmmo(int32 OldMaxSecondaryClipAmmo);
};
