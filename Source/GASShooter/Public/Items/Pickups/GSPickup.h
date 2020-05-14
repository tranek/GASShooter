// Copyright 2020 Dan Kestranek.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GameplayTagContainer.h"
#include "GSPickup.generated.h"

class AGSCharacterBase;

UCLASS()
class GASSHOOTER_API AGSPickup : public AActor
{
	GENERATED_BODY()
	
public:	
	AGSPickup();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	// Pickup on touch
	virtual void NotifyActorBeginOverlap(class AActor* Other) override;

	// Check if pawn can use this pickup
	virtual bool CanBePickedUp(AGSCharacterBase* TestCharacter) const;

	UFUNCTION(BlueprintNativeEvent, Meta = (DisplayName = "CanBePickedUp"))
	bool K2_CanBePickedUp(AGSCharacterBase* TestCharacter) const;
	virtual bool K2_CanBePickedUp_Implementation(AGSCharacterBase* TestCharacter) const;

protected:
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "GSPickup")
	class UCapsuleComponent* CollisionComp;

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_IsActive, Category = "GSPickup")
	bool bIsActive;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "GSPickup")
	bool bCanRespawn;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "GSPickup")
	float RespawnTime;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "GSPickup")
	FGameplayTagContainer RestrictedPickupTags;

	// Sound played when player picks it up
	UPROPERTY(EditDefaultsOnly, Category = "GSPickup")
	class USoundCue* PickupSound;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "GSPickup")
	TArray<TSubclassOf<class UGSGameplayAbility>> AbilityClasses;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "GSPickup")
	TArray<TSubclassOf<class UGameplayEffect>> EffectClasses;

	// The character who has picked up this pickup
	UPROPERTY(BlueprintReadOnly, Replicated)
	AGSCharacterBase* PickedUpBy;

	FTimerHandle TimerHandle_RespawnPickup;

	void PickupOnTouch(AGSCharacterBase* Pawn);

	virtual void GivePickupTo(AGSCharacterBase* Pawn);

	// Show effects when pickup disappears
	virtual void OnPickedUp();

	// Blueprint implementable effects
	UFUNCTION(BlueprintImplementableEvent, Meta = (DisplayName = "OnPickedUp"))
	void K2_OnPickedUp();

	virtual void RespawnPickup();

	// Show effects when pickup appears
	virtual void OnRespawned();

	// Blueprint implementable effects
	UFUNCTION(BlueprintImplementableEvent, Meta = (DisplayName = "OnRespawned"))
	void K2_OnRespawned();

	UFUNCTION()
	virtual void OnRep_IsActive();
};
