// Copyright 2019 Dan Kestranek.

#pragma once

#include "CoreMinimal.h"
#include "GameplayAbilitySpec.h"
#include "GameFramework/Actor.h"
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

	UFUNCTION(BlueprintCallable, Category = "GASShooter|GSWeapon")
	virtual USkeletalMeshComponent* GetWeaponMesh1P();

	UFUNCTION(BlueprintCallable, Category = "GASShooter|GSWeapon")
	virtual USkeletalMeshComponent* GetWeaponMesh3P();

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

	UPROPERTY(VisibleAnywhere, Category = "GASShooter|GSWeapon")
	UAnimMontage* EquipMontage;

	UPROPERTY(VisibleAnywhere, Category = "GASShooter|GSWeapon")
	UAnimMontage* ReloadMontage;

	UPROPERTY(VisibleAnywhere, Category = "GASShooter|GSWeapon")
	UAnimMontage* FireMontage;

	UPROPERTY(EditAnywhere, Category = "GASShooter|GSWeapon")
	TArray<TSubclassOf<UGSGameplayAbility>> Abilities;

	TArray<FGameplayAbilitySpecHandle> AbilitySpecs;

	// Called when the player picks up this weapon
	virtual void PickUpOnTouch(AGSHeroCharacter* InCharacter);

	virtual void AddAbilities();

	virtual void RemoveAbilities();
};
