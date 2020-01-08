// Copyright 2020 Dan Kestranek.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GameplayAbilitySpec.h"
#include "GASShooter/GASShooter.h"
#include "GSWeapon.generated.h"

class AGSHeroCharacter;
class UAnimMontage;
class UGSGameplayAbility;
class UPaperSprite;
class USkeletalMeshComponent;

UCLASS(Blueprintable, BlueprintType)
class GASSHOOTER_API AGSWeapon : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AGSWeapon();

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

	UFUNCTION(BlueprintCallable, Category = "GASShooter|GSWeapon")
	virtual USkeletalMeshComponent* GetWeaponMesh1P();

	UFUNCTION(BlueprintCallable, Category = "GASShooter|GSWeapon")
	virtual USkeletalMeshComponent* GetWeaponMesh3P();

	UPROPERTY(BlueprintReadWrite, VisibleInstanceOnly, Category = "GASShooter|GSWeapon")
	FGameplayTag FireMode;

	// Things like fire mode for rifle
	UPROPERTY(BlueprintReadWrite, VisibleInstanceOnly, Category = "GASShooter|GSWeapon")
	FText StatusText;

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
	virtual int32 GetClipAmmo() const;

	UFUNCTION(BlueprintCallable, Category = "GASShooter|GSWeapon")
	virtual int32 GetReserveAmmo() const;

protected:
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
