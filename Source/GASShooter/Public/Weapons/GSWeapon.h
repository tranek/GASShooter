// Copyright 2020 Dan Kestranek.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "AbilitySystemInterface.h"
#include "GameplayAbilitySpec.h"
#include "GASShooter/GASShooter.h"
#include "GSWeapon.generated.h"

class AGSHeroCharacter;
class UAnimMontage;
class UGSAbilitySystemComponent;
class UGSGameplayAbility;
class UGSWeaponAttributeSet;
class UPaperSprite;
class USkeletalMeshComponent;

UCLASS(Blueprintable, BlueprintType)
class GASSHOOTER_API AGSWeapon : public AActor, public IAbilitySystemInterface
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AGSWeapon();

	UPROPERTY(Replicated)
	UGSWeaponAttributeSet* AttributeSet;

	// This tag will be used by the Characters when they equip a weapon to gate activation of abilities
	// since abilities are granted on adding to inventory and removed when the weapon is removed from the
	// inventory - not on equip/unequip.
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "GASShooter|GSWeapon")
	FGameplayTag WeaponTag;

	UPROPERTY(BlueprintReadOnly, Category = "GASShooter|GSWeapon")
	class AGSGATA_SingleLineTrace* SingleLineTraceTargetActor;
	
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

	// Things like fire mode for rifle
	UPROPERTY(BlueprintReadWrite, VisibleInstanceOnly, Category = "GASShooter|GSWeapon")
	FText StatusText;


	/**
	* Starting Attribute values. Initialize in BeginPlay on the Server. If the weapons could level up,
	* I'd opt for a DataTable and a GameplayEffect instead or one of the advanced init functions on AttributeSet.
	* But this simple way of Initializing from the Blueprint will be fine for this.
	*/

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "GASShooter|GSWeapon|Attributes")
	int32 PrimaryMaxClipAmmo;

	// How much ammo in the clip the gun starts with
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "GASShooter|GSWeapon|Attributes")
	int32 PrimaryClipAmmo;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "GASShooter|GSWeapon|Attributes")
	int32 PrimaryMaxAmmo;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "GASShooter|GSWeapon|Attributes")
	int32 PrimaryReserveAmmo;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "GASShooter|GSWeapon|Attributes")
	int32 PrimaryDamage;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "GASShooter|GSWeapon|Attributes")
	int32 SecondaryMaxClipAmmo;

	// How much ammo in the clip the gun starts with
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "GASShooter|GSWeapon|Attributes")
	int32 SecondaryClipAmmo;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "GASShooter|GSWeapon|Attributes")
	int32 SecondaryMaxAmmo;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "GASShooter|GSWeapon|Attributes")
	int32 SecondaryReserveAmmo;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "GASShooter|GSWeapon|Attributes")
	int32 SecondaryDamage;


	// Implement IAbilitySystemInterface
	virtual class UAbilitySystemComponent* GetAbilitySystemComponent() const override;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "GASShooter|GSWeapon")
	virtual USkeletalMeshComponent* GetWeaponMesh1P() const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "GASShooter|GSWeapon")
	virtual USkeletalMeshComponent* GetWeaponMesh3P() const;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	void SetOwningCharacter(AGSHeroCharacter* InOwningCharacter);

	// Pickup on touch
	virtual void NotifyActorBeginOverlap(class AActor* Other) override;

	// Called when the player equips this weapon
	virtual void Equip();

	// Called when the player unequips this weapon
	virtual void UnEquip();

	virtual void AddAbilities();

	virtual void RemoveAbilities();

	// Reloads the weapon
	virtual void Reload();

	virtual int32 GetAbilityLevel(EGSAbilityInputID AbilityID);

	// Resets things like fire mode to default
	UFUNCTION(BlueprintCallable, Category = "GASShooter|GSWeapon")
	virtual void ResetWeapon();

	UFUNCTION(BlueprintCallable, Category = "GASShooter|GSWeapon")
	virtual int32 GetPrimaryClipAmmo() const;

	UFUNCTION(BlueprintCallable, Category = "GASShooter|GSWeapon")
	virtual int32 GetPrimaryMaxClipAmmo() const;

	UFUNCTION(BlueprintCallable, Category = "GASShooter|GSWeapon")
	virtual int32 GetPrimaryReserveAmmo() const;

	UFUNCTION(BlueprintCallable, Category = "GASShooter|GSWeapon")
	virtual int32 GetSecondaryClipAmmo() const;

	UFUNCTION(BlueprintCallable, Category = "GASShooter|GSWeapon")
	virtual int32 GetSecondaryMaxClipAmmo() const;

	UFUNCTION(BlueprintCallable, Category = "GASShooter|GSWeapon")
	virtual int32 GetSecondaryReserveAmmo() const;

protected:
	UPROPERTY()
	UGSAbilitySystemComponent* AbilitySystemComponent;

	// Generic Root component so that we can hide visibility of one mesh without affecting the other if they were parent/child
	UPROPERTY(VisibleAnywhere)
	USceneComponent* Root;

	UPROPERTY(VisibleAnywhere, Category = "GASShooter|GSWeapon")
	USkeletalMeshComponent* WeaponMesh1P;

	UPROPERTY(VisibleAnywhere, Category = "GASShooter|GSWeapon")
	USkeletalMeshComponent* WeaponMesh3P;

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

	// Cache tags
	FGameplayTag WeaponPrimaryInstantAbilityTag;
	FGameplayTag WeaponSecondaryInstantAbilityTag;
	FGameplayTag WeaponAlternateInstantAbilityTag;

	virtual void BeginPlay() override;

	// Called when the player picks up this weapon
	virtual void PickUpOnTouch(AGSHeroCharacter* InCharacter);

};
