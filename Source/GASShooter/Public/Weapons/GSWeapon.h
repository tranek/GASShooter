// Copyright 2019 Dan Kestranek.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GameplayAbilitySpec.h"
#include "GASShooter/GASShooter.h"
#include "GSWeapon.generated.h"

class AGSHeroCharacter;
class UAnimMontage;
class UGSGameplayAbility;
class USkeletalMeshComponent;

UCLASS(Blueprintable, BlueprintType)
class GASSHOOTER_API AGSWeapon : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AGSWeapon();

	// Cached batchable primary instant ability spec handle
	UPROPERTY(BlueprintReadOnly, Category = "GASShooter|GSWeapon")
	FGameplayAbilitySpecHandle PrimaryInstantAbilitySpecHandle;

	// Cached batchable primary instant ability spec
	FGameplayAbilitySpec* PrimaryInstantAbilitySpec;

	// Cached batchable primary instant ability
	UPROPERTY(BlueprintReadOnly, Category = "GASShooter|GSWeapon")
	UGSGameplayAbility* PrimaryInstantAbility;

	// Cached batchable secondary instant ability spec handle
	UPROPERTY(BlueprintReadOnly, Category = "GASShooter|GSWeapon")
	FGameplayAbilitySpecHandle SecondaryInstantAbilitySpecHandle;

	// Cached batchable secondary instant ability spec
	FGameplayAbilitySpec* SecondaryInstantAbilitySpec;

	// Cached batchable secondary instant ability
	UPROPERTY(BlueprintReadOnly, Category = "GASShooter|GSWeapon")
	UGSGameplayAbility* SecondaryInstantAbility;

	// Cached batchable alternate instant ability spec handle
	UPROPERTY(BlueprintReadOnly, Category = "GASShooter|GSWeapon")
	FGameplayAbilitySpecHandle AlternateInstantAbilitySpecHandle;

	// Cached batchable alternate instant ability spec
	FGameplayAbilitySpec* AlternateInstantAbilitySpec;

	// Cached batchable alternate instant ability
	UPROPERTY(BlueprintReadOnly, Category = "GASShooter|GSWeapon")
	UGSGameplayAbility* AlternateInstantAbility;

	UPROPERTY(BlueprintReadOnly, Category = "GASShooter|GSWeapon")
	class AGSGATA_SingleLineTrace* SingleLineTraceTargetActor;

	UFUNCTION(BlueprintCallable, Category = "GASShooter|GSWeapon")
	virtual USkeletalMeshComponent* GetWeaponMesh1P();

	UFUNCTION(BlueprintCallable, Category = "GASShooter|GSWeapon")
	virtual USkeletalMeshComponent* GetWeaponMesh3P();

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "GASShooter|GSWeapon")
	FGameplayTag FireMode;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	void SetOwningCharacter(AGSHeroCharacter* InOwningCharacter);

	// Pickup on touch
	virtual void NotifyActorBeginOverlap(class AActor* Other) override;

	// Called when the player equips this weapon
	virtual void Equip();

	// Called when the player unequips this weapon
	virtual void UnEquip();

	// Reloads the weapon
	virtual void Reload();

	virtual int32 GetAbilityLevel(EGSAbilityInputID AbilityID);

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

	TArray<FGameplayAbilitySpecHandle> AbilitySpecHandles;

	// Cache tags
	FGameplayTag WeaponPrimaryInstantAbilityTag;
	FGameplayTag WeaponSecondaryInstantAbilityTag;
	FGameplayTag WeaponAlternateInstantAbilityTag;

	virtual void BeginPlay() override;

	// Called when the player picks up this weapon
	virtual void PickUpOnTouch(AGSHeroCharacter* InCharacter);

	virtual void AddAbilities();

	virtual void RemoveAbilities();

	virtual void HandleAbilityGiven(FGameplayAbilitySpec& AbilitySpec);
};
